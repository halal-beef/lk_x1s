/*
 * Copyright (c) 2009-2015 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <stdlib.h>
#include <debug.h>
#include <trace.h>
#include <err.h>
#include <string.h>
#include <assert.h>
#include <list.h>
#include <pow2.h>
#include <lib/bio.h>
#include <kernel/mutex.h>
#include <lk/init.h>

#define LOCAL_TRACE 0
#define BYTE_TO_BLOCK(x) (((x - 1) >> 9) + 1)

static struct {
    struct list_node list;
    mutex_t lock;
} bdevs = {
    .list = LIST_INITIAL_VALUE(bdevs.list),
    .lock = MUTEX_INITIAL_VALUE(bdevs.lock),
};

/* default implementation is to use the read_block hook to 'deblock' the device */
static ssize_t bio_default_read(struct bdev *dev, void *_buf, off_t offset, size_t len)
{
    uint8_t *buf = (uint8_t *)_buf;
    ssize_t bytes_read = 0;
    bnum_t block;
    ssize_t err = 0;
    STACKBUF_DMA_ALIGN(temp, dev->block_size); // temporary buffer for partial block transfers

    /* find the starting block */
    block = offset / dev->block_size;

    LTRACEF("buf %p, offset %lld, block %u, len %zd\n", buf, offset, block, len);
    /* handle partial first block */
    if ((offset % dev->block_size) != 0) {
        /* read in the block */
        err = bio_read_block(dev, temp, block, 1);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size) {
            err = ERR_IO;
            goto err;
        }

        /* copy what we need */
        size_t block_offset = offset % dev->block_size;
        size_t tocopy = MIN(dev->block_size - block_offset, len);
        memcpy(buf, temp + block_offset, tocopy);

        /* increment our buffers */
        buf += tocopy;
        len -= tocopy;
        bytes_read += tocopy;
        block++;
    }

    LTRACEF("buf %p, block %u, len %zd\n", buf, block, len);

    // If the device requires alignment AND our buffer is not alread aligned.
    bool requires_alignment =
        (dev->flags & BIO_FLAG_CACHE_ALIGNED_READS) &&
        (IS_ALIGNED((size_t)buf, CACHE_LINE) == false);
    /* handle middle blocks */
    if (requires_alignment) {
        while (len >= dev->block_size) {
            /* do the middle reads */
            err = bio_read_block(dev, temp, block, 1);
            if (err < 0) {
                goto err;
            } else if ((size_t)err != dev->block_size) {
                err = ERR_IO;
                goto err;
            }
            memcpy(buf, temp, dev->block_size);

            buf += dev->block_size;
            len -= dev->block_size;
            bytes_read += dev->block_size;
            block++;
        }
    } else {
        uint32_t num_blocks = divpow2(len, dev->block_shift);
        err = bio_read_block(dev, buf, block, num_blocks);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size * num_blocks) {
            err = ERR_IO;
            goto err;
        }
        buf += err;
        len -= err;
        bytes_read += err;
        block += num_blocks;
    }

    LTRACEF("buf %p, block %u, len %zd\n", buf, block, len);
    /* handle partial last block */
    if (len > 0) {
        /* read the block */
        err = bio_read_block(dev, temp, block, 1);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size) {
            err = ERR_IO;
            goto err;
        }

        /* copy the partial block from our temp buffer */
        memcpy(buf, temp, len);

        bytes_read += len;
    }

err:
    /* return error or bytes read */
    return (err >= 0) ? bytes_read : err;
}

static ssize_t bio_default_write(struct bdev *dev, const void *_buf, off_t offset, size_t len)
{
    const uint8_t *buf = (const uint8_t *)_buf;
    ssize_t bytes_written = 0;
    bnum_t block;
    ssize_t err = 0;
    STACKBUF_DMA_ALIGN(temp, dev->block_size); // temporary buffer for partial block transfers

    /* find the starting block */
    block = offset / dev->block_size;

    LTRACEF("buf %p, offset %lld, block %u, len %zd\n", buf, offset, block, len);
    /* handle partial first block */
    if ((offset % dev->block_size) != 0) {
        /* read in the block */
        err = bio_read_block(dev, temp, block, 1);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size) {
            err = ERR_IO;
            goto err;
        }

        /* copy what we need */
        size_t block_offset = offset % dev->block_size;
        size_t tocopy = MIN(dev->block_size - block_offset, len);
        memcpy(temp + block_offset, buf, tocopy);

        /* write it back out */
        err = bio_write_block(dev, temp, block, 1);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size) {
            err = ERR_IO;
            goto err;
        }

        /* increment our buffers */
        buf += tocopy;
        len -= tocopy;
        bytes_written += tocopy;
        block++;
    }

    LTRACEF("buf %p, block %u, len %zd\n", buf, block, len);

    // If the device requires alignment AND our buffer is not alread aligned.
    bool requires_alignment =
        (dev->flags & BIO_FLAG_CACHE_ALIGNED_WRITES) &&
        (IS_ALIGNED((size_t)buf, CACHE_LINE) == false);

    /* handle middle blocks */
    if (requires_alignment) {
        while (len >= dev->block_size) {
            /* do the middle reads */
            memcpy(temp, buf, dev->block_size);
            err = bio_write_block(dev, temp, block, 1);
            if (err < 0) {
                goto err;
            } else if ((size_t)err != dev->block_size) {
                err = ERR_IO;
                goto err;
            }

            buf += dev->block_size;
            len -= dev->block_size;
            bytes_written += dev->block_size;
            block++;
        }
    } else {
        uint32_t block_count = divpow2(len, dev->block_shift);
        err = bio_write_block(dev, buf, block, block_count);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size * block_count) {
            err = ERR_IO;
            goto err;
        }

        DEBUG_ASSERT((size_t)err == (block_count * dev->block_size));

        buf += err;
        len -= err;
        bytes_written += err;
        block += block_count;
    }

    LTRACEF("buf %p, block %u, len %zd\n", buf, block, len);
    /* handle partial last block */
    if (len > 0) {
        /* read the block */
        err = bio_read_block(dev, temp, block, 1);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size) {
            err = ERR_IO;
            goto err;
        }

        /* copy the partial block from our temp buffer */
        memcpy(temp, buf, len);

        /* write it back out */
        err = bio_write_block(dev, temp, block, 1);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size) {
            err = ERR_IO;
            goto err;
        }

        bytes_written += len;
    }

err:
    /* return error or bytes written */
    return (err >= 0) ? bytes_written : err;
}

static ssize_t bio_default_erase(struct bdev *dev, off_t offset, size_t len)
{
    /* default erase operation is to just write zeros over the device */
    STACKBUF_DMA_ALIGN(erase_buf, dev->block_size);

    memset(erase_buf, dev->erase_byte, dev->block_size);

    ssize_t erased = 0;
    size_t remaining = len;
    off_t pos = offset;
    while (remaining > 0) {
        size_t towrite = MIN(remaining, dev->block_size);

        ssize_t written = bio_write(dev, erase_buf, pos, towrite);
        if (written < 0)
            return written;

        erased += written;
        pos += written;
        remaining -= written;

        if ((size_t)written < towrite)
            break;
    }

    return erased;
}

static ssize_t bio_default_read_block(struct bdev *dev, void *buf, bnum_t block, uint count)
{
    return ERR_NOT_SUPPORTED;
}

static ssize_t bio_default_write_block(struct bdev *dev, const void *buf, bnum_t block, uint count)
{
    return ERR_NOT_SUPPORTED;
}

static uint bio_new_read(struct bdev *dev, void *_buf, bnum_t _block, uint count)
{
	uint8_t *buf = (uint8_t *)_buf;
	bnum_t block = _block;
	uint block_read = 0;
	uint native_block_size = dev->block_size / USER_BLOCK_SIZE;
	uint block_per_time;
	ssize_t byte_size;
	ssize_t byte_offset;
	STACKBUF_DMA_ALIGN(temp, dev->block_size);
	uint max_blkcnt = (dev->max_blkcnt_per_cmd) ? dev->max_blkcnt_per_cmd : 32;
	uint p_cnt;


	/* Not support for cache alignment */
	if ((dev->flags & BIO_FLAG_CACHE_ALIGNED_READS) &&
			(IS_ALIGNED((size_t)buf, CACHE_LINE) == false)) {
		printf("Not support for cache alignement !!\n");
		goto end;
	}

	/*
	 * handle partial first block
	 *
	 * Each device driver's maximum size can exist when it executes
	 * a read command. So we determine what the size is and
	 * should use less and equal than the size
	 * every time we issue a read command
	 */
	if (((block % native_block_size) != 0) || count < native_block_size) {
		/* Read one native block */
		block = (block / native_block_size) * native_block_size;
		if (dev->new_read_native(dev, temp, block / native_block_size, 1))
			goto end;

		/* Copy to original buffer */
		byte_offset = (_block - block) * USER_BLOCK_SIZE;
		p_cnt = MIN(count, block + native_block_size - _block);
		byte_size = p_cnt * USER_BLOCK_SIZE;
		memcpy(buf, temp + byte_offset, byte_size);

		block_read += p_cnt;
		buf += byte_size;
		block += native_block_size;
	}

	/* Loop */
	do {
		block_per_time = count - block_read;
		if (block_per_time) {
			if (block_per_time >= max_blkcnt)
				block_per_time = max_blkcnt;
		} else
			goto end;

		if (dev->new_read_native(dev, buf, block / native_block_size,
					block_per_time / native_block_size))
			goto end;

		block_read += block_per_time;
		buf += block_per_time * USER_BLOCK_SIZE;
		block += block_per_time;

		if (count - block_read < native_block_size)
			break;

	} while (1);


	/* handle partial last block */
	if (((count - block_read) % native_block_size) != 0) {
		if (dev->new_read_native(dev, temp, block / native_block_size, 1))
			goto end;

		byte_size = (count - block_read) * USER_BLOCK_SIZE;
		memcpy(buf, temp, byte_size);

		block_read += count - block_read;
	}
end:
	return block_read;
}

static uint bio_new_write(struct bdev *dev, const void *_buf, bnum_t _block, uint count)
{
	uint8_t *buf = (uint8_t *)_buf;
	bnum_t block = _block;
	uint block_written = 0;
	uint native_block_size = dev->block_size / USER_BLOCK_SIZE;
	uint block_per_time;
	ssize_t byte_size;
	ssize_t byte_offset;
	STACKBUF_DMA_ALIGN(temp, dev->block_size);
	uint max_blkcnt = (dev->max_blkcnt_per_cmd) ? dev->max_blkcnt_per_cmd : 32;
	uint p_cnt;


	/* Not support for cache alignment */
	if ((dev->flags & BIO_FLAG_CACHE_ALIGNED_WRITES) &&
			(IS_ALIGNED((size_t)buf, CACHE_LINE) == false)) {
		printf("Not support for cache alignement !!\n");
		goto end;
	}

	/*
	 * handle partial first block
	 *
	 * Each device driver's maximum size can exist when it executes
	 * a write command. So we determine what the size is and
	 * should use less and equal than the size
	 * every time we issue a write command
	 */
	if ((block % native_block_size) != 0) {
		/* Read one native block */
		block = (block / native_block_size) * native_block_size;
		if (dev->new_read_native(dev, temp, block / native_block_size, 1))
			goto end;

		/* Modity temporal buffer */
		byte_offset = (_block - block) * USER_BLOCK_SIZE;
		p_cnt = MIN(count, block + native_block_size - _block);
		byte_size = p_cnt * USER_BLOCK_SIZE;
		memcpy(temp + byte_offset, buf, byte_size);

		/* Write one native block */
		if (dev->new_write_native(dev, temp, block / native_block_size, 1))
			goto end;

		block_written += p_cnt;
		buf += byte_size;
		block += native_block_size;
	}

	/* Loop */
	do {
		block_per_time = count - block_written;
		if (block_per_time) {
			if (block_per_time >= max_blkcnt)
				block_per_time = max_blkcnt;
		} else
			goto end;

		if (dev->new_write_native(dev, buf, block / native_block_size,
					block_per_time / native_block_size))
			goto end;

		block_written += block_per_time;
		buf += block_per_time * USER_BLOCK_SIZE;
		block += block_per_time;

		if (count - block_written < native_block_size)
			break;

	} while (1);

	/* handle partial last block */
	if (((count - block_written) % native_block_size) != 0) {
		/* Read one native block */
		if (dev->new_read_native(dev, temp, block / native_block_size, 1))
			goto end;

		/* Modity temporal buffer */
		byte_size = (count - block_written) * USER_BLOCK_SIZE;
		memcpy(temp, buf, byte_size);

		/* Write one native block */
		if (dev->new_write_native(dev, temp, block / native_block_size, 1))
			goto end;

		block_written += count - block_written;
	}
end:
	return block_written;
}

static uint bio_new_byte_read(struct bdev *dev, void *_buf, bnum_t _block, uint size)
{
	uint8_t *buf = (uint8_t *)_buf;
	bnum_t block = _block;	/*start block address*/
	uint native_block_size = dev->block_size / USER_BLOCK_SIZE; /*ufs = 8 */
	uint block_per_time;
	uint count = BYTE_TO_BLOCK(size); /* size to block*/
	uint block_read = count;
	uint rsize = size;
	ssize_t byte_size;
	ssize_t byte_offset;
	STACKBUF_DMA_ALIGN(temp, dev->block_size);
	uint max_blkcnt = (dev->max_blkcnt_per_cmd) ? dev->max_blkcnt_per_cmd : 32;
	uint p_cnt;


	/* Not support for cache alignment */
	if ((dev->flags & BIO_FLAG_CACHE_ALIGNED_READS) &&
			(IS_ALIGNED((size_t)buf, CACHE_LINE) == false)) {
		printf("Not support for cache alignement !!\n");
		goto end;
	}

	/*
	 * handle partial first block
	 *
	 * Each device driver's maximum size can exist when it executes
	 * a read command. So we determine what the size is and
	 * should use less and equal than the size
	 * every time we issue a read command
	 */
	if (((block % native_block_size) != 0) || count < native_block_size) {
		/* Read one native block */
		block = (block / native_block_size) * native_block_size;
		if (dev->new_read_native(dev, temp, block / native_block_size, 1))
			goto end;

		/* Copy to original buffer */
		byte_offset = (_block - block) * USER_BLOCK_SIZE;
		p_cnt = MIN(count, block + native_block_size - _block);
		byte_size = MIN(size, p_cnt * USER_BLOCK_SIZE);
		memcpy(buf, temp + byte_offset, byte_size);

		block = _block;
		block_read -= p_cnt;
		buf += byte_size;
		block += p_cnt;
		rsize -= byte_size;
	}

	while (block_read >= native_block_size) {
		block_per_time = (((block_read > max_blkcnt ? max_blkcnt : block_read)
					/ native_block_size) * native_block_size);

		if (dev->new_read_native(dev, buf, block / native_block_size,
					block_per_time / native_block_size))
			goto end;

		block_read -= block_per_time;
		buf += block_per_time * USER_BLOCK_SIZE;
		block += block_per_time;
		rsize -= block_per_time * USER_BLOCK_SIZE;
	}

	/* handle partial last block */
	if (block_read) {
		if (dev->new_read_native(dev, temp, block / native_block_size, 1))
			goto end;
		memcpy(buf, temp, rsize);
		block_read -= 1;
		rsize = 0;
	}
end:
	return count - block_read;
}

static uint bio_new_byte_write(struct bdev *dev, const void *_buf, bnum_t _block, uint size)
{
	uint8_t *buf = (uint8_t *)_buf;
	bnum_t block = _block;
	uint native_block_size = dev->block_size / USER_BLOCK_SIZE;
	uint block_per_time;
	uint count = BYTE_TO_BLOCK(size);
	uint block_written = count;
	uint wsize = size;
	ssize_t byte_size;
	ssize_t byte_offset;
	STACKBUF_DMA_ALIGN(temp, dev->block_size);
	uint max_blkcnt = (dev->max_blkcnt_per_cmd) ? dev->max_blkcnt_per_cmd : 32;
	uint p_cnt;


	/* Not support for cache alignment */
	if ((dev->flags & BIO_FLAG_CACHE_ALIGNED_WRITES) &&
			(IS_ALIGNED((size_t)buf, CACHE_LINE) == false)) {
		printf("Not support for cache alignement !!\n");
		goto end;
	}

	/*
	 * handle partial first block
	 *
	 * Each device driver's maximum size can exist when it executes
	 * a write command. So we determine what the size is and
	 * should use less and equal than the size
	 * every time we issue a write command
	 */
	if ((block % native_block_size) != 0) {
		/* Read one native block */
		block = (block / native_block_size) * native_block_size;
		if (dev->new_read_native(dev, temp, block / native_block_size, 1))
			goto end;

		/* Modity temporal buffer */
		byte_offset = (_block - block) * USER_BLOCK_SIZE;
		p_cnt = MIN(count, block + native_block_size - _block);
		byte_size = MIN(size, p_cnt * USER_BLOCK_SIZE);
		memcpy(temp + byte_offset, buf, byte_size);

		/* Write one native block */
		if (dev->new_write_native(dev, temp, block / native_block_size, 1))
			goto end;

		block = _block;
		block_written -= p_cnt;
		buf += byte_size;
		block += p_cnt;
		wsize -= byte_size;
	}

	while (block_written >= native_block_size) {
		block_per_time = (((block_written > max_blkcnt ? max_blkcnt : block_written)
					/ native_block_size) * native_block_size);

		if (dev->new_write_native(dev, buf, block / native_block_size, block_per_time / native_block_size))
			goto end;

		block_written -= block_per_time;
		buf += block_per_time * USER_BLOCK_SIZE;
		block += block_per_time;
		wsize -= block_per_time * USER_BLOCK_SIZE;
	}

	/* handle partial last block */
	if (block_written) {
		/* Read one native block */
		if (dev->new_read_native(dev, temp, block / native_block_size, 1))
			goto end;

		/* Modity temporal buffer */
		memcpy(temp, buf, wsize);

		/* Write one native block */
		if (dev->new_write_native(dev, temp, block / native_block_size, 1))
			goto end;

		block_written -= 1;
		wsize = 0;
	}
end:
	return (count - block_written);
}

static uint bio_new_erase(struct bdev *dev, bnum_t _block, uint count)
{
	bnum_t block = _block;
	uint block_erased = 0;
	uint native_block_size = dev->block_size / USER_BLOCK_SIZE;
	uint block_per_time;
	uint erase_size;
	ssize_t byte_size;
	ssize_t byte_offset;
	STACKBUF_DMA_ALIGN(temp, dev->block_size);
	/*uint max_blkcnt = (dev->max_blkcnt_per_cmd) ? dev->max_blkcnt_per_cmd : 32;*/
	uint p_cnt;

	if (dev->erase_size &&
		dev->erase_size > native_block_size &&
		dev->erase_size % native_block_size == 0)
		erase_size = dev->erase_size / USER_BLOCK_SIZE;
	else
		erase_size = native_block_size;
	/*
	 * handle partial first block
	 *
	 * Each device driver's maximum size can exist when it executes
	 * a write command. So we determine what the size is and
	 * should use less and equal than the size
	 * every time we issue a write command
	 */
	if ((block % native_block_size) != 0) {
		/* Read one native block */
		block = (block / native_block_size) * native_block_size;
		if (dev->new_read_native(dev, temp, block / native_block_size, 1))
			goto end;

		/* Clear temporal buffer partially */
		byte_offset = (_block - block) * USER_BLOCK_SIZE;
		p_cnt = MIN(count, block + native_block_size - _block);
		byte_size = p_cnt * USER_BLOCK_SIZE;
		memset(temp + byte_offset, 0, byte_size);

		/* Write one native block */
		if (dev->new_write_native(dev, temp, block / native_block_size, 1))
			goto end;

		block_erased += p_cnt;
		block += native_block_size;
	}

	memset(temp, 0, dev->block_size);
	while (block % erase_size != 0 &&
		(block_erased + native_block_size) <= count) {
		dev->new_write_native(dev, temp, block/native_block_size, 1);
		block += native_block_size;
		block_erased += native_block_size;
	}
	block_per_time = count - block_erased;
	block_per_time = (block_per_time / erase_size) * erase_size;
	if (block_per_time) {
		if (dev->new_erase_native(dev, block / native_block_size,
					block_per_time / native_block_size))
			goto end;

		block_erased += block_per_time;
		block += block_per_time;
	}

	/* All the blocks consumed */
	if (_block + count == block)
		goto end;

	memset(temp, 0, dev->block_size);
	while ((block_erased + native_block_size) <= count){
		dev->new_write_native(dev, temp, block/native_block_size, 1);
		block += native_block_size;
		block_erased += native_block_size;
	}

	/* handle partial last block */
	if (((count - block_erased) % native_block_size) != 0) {
		/* Read one native block */
		if (dev->new_read_native(dev, temp, block / native_block_size, 1))
			goto end;

		/* Modity temporal buffer */
		byte_size = (count - block_erased) * USER_BLOCK_SIZE;
		memset(temp, 0, byte_size);

		/* Write one native block */
		if (dev->new_write_native(dev, temp, block / native_block_size, 1))
			goto end;

		block_erased += count - block_erased;
	}
end:
	return block_erased;

}

static void bdev_inc_ref(bdev_t *dev)
{
    LTRACEF("Add ref \"%s\" %d -> %d\n", dev->name, dev->ref, dev->ref + 1);
    //atomic_add(&dev->ref, 1);
    dev->ref++;;
}

static void bdev_dec_ref(bdev_t *dev)
{
    //int oldval = atomic_add(&dev->ref, -1);
    int oldval = (dev->ref--);

    LTRACEF("Dec ref \"%s\" %d -> %d\n", dev->name, oldval, dev->ref);

    if (oldval == 1) {
        // last ref, remove it
        //DEBUG_ASSERT(!list_in_list(&dev->node));

        TRACEF("last ref, removing (%s)\n", dev->name);

        // call the close hook if it exists
        if (dev->close)
            dev->close(dev);

        free(dev->name);
    }
}

size_t bio_trim_range(const bdev_t *dev, off_t offset, size_t len)
{
    /* range check */
    if (offset < 0)
        return 0;
    if (offset >= dev->total_size)
        return 0;
    if (len == 0)
        return 0;
    if ((off_t)(offset + len) > dev->total_size)
        len = dev->total_size - offset;

    return len;
}

uint bio_trim_block_range(const bdev_t *dev, bnum_t block, uint count)
{
    if (block > dev->block_count)
        return 0;
    if (count == 0)
        return 0;
    if (block + count > dev->block_count)
        count = dev->block_count - block;

    return count;
}

bdev_t *bio_open(const char *name)
{
    bdev_t *bdev = NULL;

    LTRACEF(" '%s'\n", name);

    /* see if it's in our list */
    bdev_t *entry;
    mutex_acquire(&bdevs.lock);
    list_for_every_entry(&bdevs.list, entry, bdev_t, node) {
        DEBUG_ASSERT(entry->ref > 0);
        if (!strcmp(entry->name, name)) {
            bdev = entry;
            bdev_inc_ref(bdev);
            break;
        }
    }
    mutex_release(&bdevs.lock);

    return bdev;
}

bdev_t *bio_get_with_prefix(const char *name)
{
    bdev_t *bdev = NULL;

    LTRACEF(" '%s'\n", name);

    /* see if it's in our list */
    bdev_t *entry;
    mutex_acquire(&bdevs.lock);
    list_for_every_entry(&bdevs.list, entry, bdev_t, node) {
        if (!strncmp(entry->name, name, strlen(name))) {
            bdev = entry;
            break;
        }
    }
    mutex_release(&bdevs.lock);

    return bdev;
}

void bio_close(bdev_t *dev)
{
    DEBUG_ASSERT(dev);
    LTRACEF(" '%s'\n", dev->name);
    bdev_dec_ref(dev);
}

ssize_t bio_read(bdev_t *dev, void *buf, off_t offset, size_t len)
{
    LTRACEF("dev '%s', buf %p, offset %lld, len %zd\n", dev->name, buf, offset, len);

    DEBUG_ASSERT(dev && dev->ref > 0);
    DEBUG_ASSERT(buf);

    /* range check */
    len = bio_trim_range(dev, offset, len);
    if (len == 0)
        return 0;

    return dev->read(dev, buf, offset, len);
}

ssize_t bio_read_block(bdev_t *dev, void *buf, bnum_t block, uint count)
{
    LTRACEF("dev '%s', buf %p, block %d, count %u\n", dev->name, buf, block, count);

    DEBUG_ASSERT(dev && dev->ref > 0);
    DEBUG_ASSERT(buf);

    /* range check */
    count = bio_trim_block_range(dev, block, count);
    if (count == 0)
        return 0;

    return dev->read_block(dev, buf, block, count);
}

ssize_t bio_write(bdev_t *dev, const void *buf, off_t offset, size_t len)
{
    LTRACEF("dev '%s', buf %p, offset %lld, len %zd\n", dev->name, buf, offset, len);

    DEBUG_ASSERT(dev && dev->ref > 0);
    DEBUG_ASSERT(buf);

    /* range check */
    len = bio_trim_range(dev, offset, len);
    if (len == 0)
        return 0;

    return dev->write(dev, buf, offset, len);
}

ssize_t bio_write_block(bdev_t *dev, const void *buf, bnum_t block, uint count)
{
    LTRACEF("dev '%s', buf %p, block %d, count %u\n", dev->name, buf, block, count);

    DEBUG_ASSERT(dev && dev->ref > 0);
    DEBUG_ASSERT(buf);

    /* range check */
    count = bio_trim_block_range(dev, block, count);
    if (count == 0)
        return 0;

    return dev->write_block(dev, buf, block, count);
}

ssize_t bio_erase(bdev_t *dev, off_t offset, size_t len)
{
    LTRACEF("dev '%s', offset %lld, len %zd\n", dev->name, offset, len);

    DEBUG_ASSERT(dev && dev->ref > 0);

    /* range check */
    len = bio_trim_range(dev, offset, len);
    if (len == 0)
        return 0;

    return dev->erase(dev, offset, len);
}

int bio_ioctl(bdev_t *dev, int request, void *argp)
{
    LTRACEF("dev '%s', request %08x, argp %p\n", dev->name, request, argp);

    if (dev->ioctl == NULL) {
        return ERR_NOT_SUPPORTED;
    } else {
        return dev->ioctl(dev, request, argp);
    }
}

void bio_initialize_bdev(bdev_t *dev,
                         const char *name,
                         size_t block_size,
                         bnum_t block_count,
                         size_t geometry_count,
                         const bio_erase_geometry_info_t *geometry,
                         const uint32_t flags)
{
    DEBUG_ASSERT(dev);
    DEBUG_ASSERT(name);

    // Block size must be finite powers of 2
    DEBUG_ASSERT(block_size && ispow2(block_size));

    list_clear_node(&dev->node);
    dev->name = strdup(name);
    dev->block_size = block_size;
    dev->block_count = block_count;
    dev->block_shift = log2_uint(block_size);
    dev->total_size = (off_t)block_count << dev->block_shift;
    dev->geometry_count = geometry_count;
    dev->geometry = geometry;
    dev->erase_byte = 0;
    dev->erase_size = 0;
    dev->ref = 0;
    dev->flags = flags;

#if DEBUG
    // If we have been supplied information about our erase geometry, sanity
    // check it in debug builds.
    if (geometry_count && geometry) {
        for (size_t i = 0; i < geometry_count; ++i) {
            bio_erase_geometry_info_t *info = geometry + i;

            // Erase sizes must be powers of two and agree with the supplied erase shift.
            DEBUG_ASSERT(info->erase_size);
            DEBUG_ASSERT(info->erase_size == ((size_t)1 << info->erase_shift));

            info->start       = desc->start;
            info->erase_size  = desc->erase_size;
            info->erase_shift = log2_uint(desc->erase_size);
            info->size        = ((off_t)desc->block_count) << desc->block_size;

            // Make sure that region is aligned on both a program and erase block boundary.
            DEBUG_ASSERT(!(info->start & (((off_t)1 << info->block_shift) - 1)));
            DEBUG_ASSERT(!(info->start & (((off_t)1 << info->erase_shift) - 1)));

            // Make sure that region's length is an integral multiple of both the
            // program and erase block size.
            DEBUG_ASSERT(!(info->size & (((off_t)1 << dev->block_shift) - 1)));
            DEBUG_ASSERT(!(info->size & (((off_t)1 << info->erase_shift) - 1)));
        }

        // Make sure that none of the regions overlap each other and that they are
        // listed in ascending order.
        for (size_t i = 0; (i + 1) < geometry_count; ++i) {
            bio_geometry_info_t *r1 = dev->geometry + i;
            bio_geometry_info_t *r2 = dev->geometry + i + 1;
            DEBUG_ASSERT(r1->start <= r2->start);

            for (size_t j = (i + 1); j < geometry_count; ++j) {
                bio_geometry_info_t *r2 = dev->geometry + j;
                DEBUG_ASSERT(!bio_does_overlap(r1->start, r1->size, r2->start, r2->size));
            }
        }
    }
#endif

    /* set up the default hooks, the sub driver should override the block operations at least */
    dev->read = bio_default_read;
    dev->read_block = bio_default_read_block;
    dev->write = bio_default_write;
    dev->write_block = bio_default_write_block;
    dev->erase = bio_default_erase;
    dev->new_read = bio_new_read;
    dev->new_write = bio_new_write;
    dev->new_erase = bio_new_erase;
    dev->new_byte_read = bio_new_byte_read;
    dev->new_byte_write = bio_new_byte_write;
    dev->close = NULL;
}

void bio_register_device(bdev_t *dev)
{
    DEBUG_ASSERT(dev);

    LTRACEF(" '%s'\n", dev->name);

    bdev_inc_ref(dev);

    mutex_acquire(&bdevs.lock);
    list_add_tail(&bdevs.list, &dev->node);
    mutex_release(&bdevs.lock);
}

void bio_unregister_device(bdev_t *dev)
{
    DEBUG_ASSERT(dev);

    LTRACEF(" '%s'\n", dev->name);

    // remove it from the list
    mutex_acquire(&bdevs.lock);
    list_delete(&dev->node);
    mutex_release(&bdevs.lock);

    bdev_dec_ref(dev); // remove the ref the list used to have
}

void bio_dump_devices(void)
{
    printf("block devices:\n");
    bdev_t *entry;
    mutex_acquire(&bdevs.lock);
    list_for_every_entry(&bdevs.list, entry, bdev_t, node) {

        printf("\t%s, size %lld, bsize %zd, ref %d",
               entry->name, entry->total_size, entry->block_size, entry->ref);

        if (!entry->geometry_count || !entry->geometry) {
            printf(" (no erase geometry)\n");
        } else {
            for (size_t i = 0; i < entry->geometry_count; ++i) {
                const bio_erase_geometry_info_t *geo = entry->geometry + i;
                printf("\n\t\terase_region[%zu] : start %lld size %lld erase size %zu",
                       i, geo->start, geo->size, geo->erase_size);

            }
        }

        printf("\n");
    }
    mutex_release(&bdevs.lock);
}
