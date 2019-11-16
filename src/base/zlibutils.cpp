#include "zlibutils.h"

#include "assert.h"

#include "zlib/zlib.h"

namespace grabbed
{
    namespace base
    {
        namespace zlibutils
        {
            bool decompress(buffer& in, base::memorystream& msOut)
            {
                assert_true(in.size() != 0);
                assert_true(msOut.getSize() != 0);

                z_stream infstream;
                infstream.zalloc = Z_NULL;
                infstream.zfree = Z_NULL;
                infstream.opaque = Z_NULL;

                // Setup out stream info
                infstream.avail_in = static_cast<uInt>(in.size());
                infstream.next_in = &in[0];
                infstream.avail_out = static_cast<uInt>(msOut.getSize());
                infstream.next_out = reinterpret_cast<Bytef*>(msOut.getData());

                int ret = inflateInit(&infstream);
                if (ret != Z_OK) {
                    assert_always("inflateInit failed");
                    return false;
                }

                ret = inflate(&infstream, Z_FINISH);
                if (ret != Z_STREAM_END)
                {
                    assert_always("inflate failed");
                    return false;
                }

                inflateEnd(&infstream);

                // kinda horrible. chunks would be better

                const bool success(infstream.total_out == msOut.getSize());

                assert_true(success);
                return success;
            }

            bool decompress(buffer& in, buffer& out)
            {
                z_stream infstream;

                infstream.zalloc = Z_NULL;
                infstream.zfree = Z_NULL;
                infstream.opaque = Z_NULL;
                infstream.avail_in = 0;
                infstream.next_in = Z_NULL;

                int ret = inflateInit(&infstream);
                if (ret != Z_OK) {
                    assert_always("inflateInit failed");
                    return false;
                }

                constexpr uInt chunkSize = 0x4000;
                unsigned char chunk[chunkSize];

                infstream.avail_in = static_cast<uInt>(in.size());
                infstream.next_in = in.data();

                /* run inflate() on input until output buffer until it says it's done*/
                do {
                    infstream.avail_out = chunkSize;
                    infstream.next_out = chunk;
                    ret = inflate(&infstream, Z_NO_FLUSH);
                    assert_true(ret != Z_STREAM_ERROR);
                    switch (ret) {
                    case Z_NEED_DICT:
                        ret = Z_DATA_ERROR;     /* and fall through */
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        (void)inflateEnd(&infstream);
                        assert_always("inflate failed");
                        break;
                    }

                    uInt have = chunkSize - infstream.avail_out;
                    size_t oldSize = out.size();

                    out.resize(oldSize + have);
                    memcpy(out.data() + oldSize, chunk, have);

                } while (ret != Z_STREAM_END);

                inflateEnd(&infstream);

                return true;
            }
        }
    }
}
