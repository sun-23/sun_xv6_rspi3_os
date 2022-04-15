/* Buffer cache.
 *
 * The buffer cache is a linked list of buf structures holding
 * cached copies of disk block contents.  Caching disk blocks
 * in memory reduces the number of disk reads and also provides
 * a synchronization point for disk blocks used by multiple processes.
 *
 * Interface:
 * * To get a buffer for a particular disk block, call bread.
 * * After changing buffer data, call bwrite to write it to disk.
 * * When done with the buffer, call brelse.
 * * Do not use the buffer after calling brelse.
 * * Only one process at a time can use a buffer,
 *     so do not keep them longer than necessary.
 *
 * The implementation uses two state flags internally:
 * * B_VALID: the buffer data has been read from the disk.
 * * B_DIRTY: the buffer data has been modified
 *     and needs to be written to disk.
 */

#include "spinlock.h"
#include "sleeplock.h"
#include "buf.h"
#include "console.h"
#include "sd.h"
#include "fs.h"

struct {
    struct spinlock lock;
    struct buf buf[NBUF];

    // Linked list of all buffers, through prev/next.
    // head.next is most recently used.
    struct buf head;
} bcache;

/* Initialize the cache list and locks. */
void
binit()
{
    /* TODO: Your code here. */
    struct buf* b;

    initlock(&bcache.lock, "bcache");

    bcache.head.prev = &bcache.head;
    bcache.head.next = &bcache.head;

    for (b = bcache.buf; b < bcache.buf + NBUF; b++) {

        b->dev = -1;
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        bcache.head.next->prev = b;
        bcache.head.next = b;

        initsleeplock(&b->lock, "buffer");
    }
}

/*
 * Look through buffer cache for block on device dev.
 * If not found, allocate a buffer.
 * In either case, return locked buffer.
 */
static struct buf *
bget(uint32_t dev, uint32_t blockno)
{
    /* TODO: Your code here. */
    struct buf* b;
    acquire(&bcache.lock);

    //from  https://github.com/sudharson14/xv6-OS-for-arm-v8/blob/master/xv6-armv8/bio.c
loop:
    for (b = bcache.head.next; b != &bcache.head; b = b->next) {

        if (b->dev == dev && b->blockno == blockno) {
            if (!holdingsleep(&b->lock)) {
                b->refcnt++;
                release(&bcache.lock);
                acquiresleep(&b->lock);
                return b;
            }
            sleep(b, &bcache.lock);
            goto loop;
        }
    }

    for (b = bcache.head.prev; b != &bcache.head; b = b->prev) {
        if ((!holdingsleep(&b->lock)) && (b->flags & B_DIRTY) == 0) {
            b->dev = dev;
            b->blockno = blockno;
            // b->flags &= ~B_VALID;//set valid bit to 0
            b->flags = 0;
            b->refcnt = 1;
            release(&bcache.lock);
            acquiresleep(&b->lock);
            return b;
        }
    }

    panic("bget: no buffers");
}

/* Return a locked buf with the contents of the indicated block. */
struct buf *
bread(uint32_t dev, uint32_t blockno)
{
    /* TODO: Your code here. */
    struct buf* b;

    b = bget(dev, blockno + 0x20800);

    if (!(b->flags & B_VALID)) {
        sdrw(b);
    }
    return b;
}

/* Write b's contents to disk. Must be locked. */
void
bwrite(struct buf *b)
{
    /* TODO: Your code here. */
     if (!holdingsleep(&b->lock))
        panic("bwrite");

    b->flags |= B_DIRTY;
    sdrw(b);
}

/*
 * Release a locked buffer.
 * Move to the head of the MRU list.
 */
void
brelse(struct buf *b)
{
    /* TODO: Your code here. */
    acquire(&bcache.lock);
    if (!holdingsleep(&b->lock))
        panic("brelse");
    releasesleep(&b->lock);

    if (--b->refcnt == 0) {
        //delete it from the list
        b->next->prev = b->prev;
        b->prev->next = b->next;

        //insert it into the head of the list
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        bcache.head.next->prev = b;
        bcache.head.next = b;
        wakeup(b);
    }
    release(&bcache.lock);
}

