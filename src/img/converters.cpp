#include "converters.h"

#include <cmath>
#include <array>

namespace grabbed
{
    namespace img
    {
        buffer ConvertToLinearTexture(const buffer& data, int _width, int _height, X360TextureFormat _textureFormat)
        {
            buffer destData(data.size());

            int blockSize;
            int texelPitch;

            switch (_textureFormat)
            {
            case X360TextureFormat::A8L8:
                blockSize = 1;
                texelPitch = 2;
                break;
            case X360TextureFormat::L8:
                blockSize = 1;
                texelPitch = 1;
                break;
            case X360TextureFormat::DXT1:
                blockSize = 4;
                texelPitch = 8;
                break;
            case X360TextureFormat::DXT3:
            case X360TextureFormat::DXT5:
            case X360TextureFormat::DXN:
                blockSize = 4;
                texelPitch = 16;
                break;
            case X360TextureFormat::A8R8G8B8:
                blockSize = 1;
                texelPitch = 4;
                break;
            case X360TextureFormat::CTX1:
                blockSize = 4;
                texelPitch = 8;
                break;
            default:
                return std::move(destData);
            }

            int blockWidth = _width / blockSize;
            int blockHeight = _height / blockSize;

            for (int j = 0; j < blockHeight; j++)
            {
                for (int i = 0; i < blockWidth; i++)
                {
                    int blockOffset = j * blockWidth + i;

                    int x = XGAddress2DTiledX(blockOffset, blockWidth, texelPitch);
                    int y = XGAddress2DTiledY(blockOffset, blockWidth, texelPitch);

                    int srcOffset = j * blockWidth * texelPitch + i * texelPitch;
                    int destOffset = y * blockWidth * texelPitch + x * texelPitch;
                    //TODO: ConvertToLinearTexture apparently breaks on on textures with a height of 64...
                    if (destOffset >= destData.size()) continue;

                    memcpy(destData.data() + destOffset, data.data() + srcOffset, texelPitch);
                }
            }

            return std::move(destData);
        }

        int XGAddress2DTiledX(int Offset, int Width, int TexelPitch)
        {
            int AlignedWidth = (Width + 31) & ~31;

            int LogBpp = (TexelPitch >> 2) + ((TexelPitch >> 1) >> (TexelPitch >> 2));
            int OffsetB = Offset << LogBpp;
            int OffsetT = ((OffsetB & ~4095) >> 3) + ((OffsetB & 1792) >> 2) + (OffsetB & 63);
            int OffsetM = OffsetT >> (7 + LogBpp);

            int MacroX = ((OffsetM % (AlignedWidth >> 5)) << 2);
            int Tile = ((((OffsetT >> (5 + LogBpp)) & 2) + (OffsetB >> 6)) & 3);
            int Macro = (MacroX + Tile) << 3;
            int Micro = ((((OffsetT >> 1) & ~15) + (OffsetT & 15)) & ((TexelPitch << 3) - 1)) >> LogBpp;

            return Macro + Micro;
        }

        int XGAddress2DTiledY(int Offset, int Width, int TexelPitch)
        {
            int AlignedWidth = (Width + 31) & ~31;

            int LogBpp = (TexelPitch >> 2) + ((TexelPitch >> 1) >> (TexelPitch >> 2));
            int OffsetB = Offset << LogBpp;
            int OffsetT = ((OffsetB & ~4095) >> 3) + ((OffsetB & 1792) >> 2) + (OffsetB & 63);
            int OffsetM = OffsetT >> (7 + LogBpp);

            int MacroY = ((OffsetM / (AlignedWidth >> 5)) << 2);
            int Tile = ((OffsetT >> (6 + LogBpp)) & 1) + (((OffsetB & 2048) >> 10));
            int Macro = (MacroY + Tile) << 3;
            int Micro = ((((OffsetT & (((TexelPitch << 6) - 1) & ~31)) + ((OffsetT & 15) << 1)) >> (3 + LogBpp)) & ~1);

            return Macro + Micro + ((OffsetT & 16) >> 4);
        }

        buffer DecodeDXT1(buffer& data, int width, int height)
        {
            buffer pixData(width*height * 4);

            int xBlocks = width / 4;
            int yBlocks = height / 4;
            for (int y = 0; y < yBlocks; y++)
            {
                for (int x = 0; x < xBlocks; x++)
                {
                    int blockDataStart = ((y * xBlocks) + x) * 8;

                    u32 color0 = ((u32)data[blockDataStart + 0] << 8) + data[blockDataStart + 1];
                    u32 color1 = ((u32)data[blockDataStart + 2] << 8) + data[blockDataStart + 3];

                    u32 code = *(u32*)(data.data() + (blockDataStart + 4));

                    u16 r0 = 0, g0 = 0, b0 = 0, r1 = 0, g1 = 0, b1 = 0;
                    r0 = (u16)(8 * (color0 & 31));
                    g0 = (u16)(4 * ((color0 >> 5) & 63));
                    b0 = (u16)(8 * ((color0 >> 11) & 31));

                    r1 = (u16)(8 * (color1 & 31));
                    g1 = (u16)(4 * ((color1 >> 5) & 63));
                    b1 = (u16)(8 * ((color1 >> 11) & 31));

                    for (int k = 0; k < 4; k++)
                    {
                        int j = k ^ 1;

                        for (int i = 0; i < 4; i++)
                        {
                            int pixDataStart = (width * (y * 4 + j) * 4) + ((x * 4 + i) * 4);
                            u32 codeDec = code & 0x3;

                            switch (codeDec)
                            {
                            case 0:
                                pixData[pixDataStart + 0] = (u8)r0;
                                pixData[pixDataStart + 1] = (u8)g0;
                                pixData[pixDataStart + 2] = (u8)b0;
                                pixData[pixDataStart + 3] = 255;
                                break;
                            case 1:
                                pixData[pixDataStart + 0] = (u8)r1;
                                pixData[pixDataStart + 1] = (u8)g1;
                                pixData[pixDataStart + 2] = (u8)b1;
                                pixData[pixDataStart + 3] = 255;
                                break;
                            case 2:
                                pixData[pixDataStart + 3] = 255;
                                if (color0 > color1)
                                {
                                    pixData[pixDataStart + 0] = (u8)((2 * r0 + r1) / 3);
                                    pixData[pixDataStart + 1] = (u8)((2 * g0 + g1) / 3);
                                    pixData[pixDataStart + 2] = (u8)((2 * b0 + b1) / 3);
                                }
                                else
                                {
                                    pixData[pixDataStart + 0] = (u8)((r0 + r1) / 2);
                                    pixData[pixDataStart + 1] = (u8)((g0 + g1) / 2);
                                    pixData[pixDataStart + 2] = (u8)((b0 + b1) / 2);
                                }
                                break;
                            case 3:
                                if (color0 > color1)
                                {
                                    pixData[pixDataStart + 0] = (u8)((r0 + 2 * r1) / 3);
                                    pixData[pixDataStart + 1] = (u8)((g0 + 2 * g1) / 3);
                                    pixData[pixDataStart + 2] = (u8)((b0 + 2 * b1) / 3);
                                    pixData[pixDataStart + 3] = 255;
                                }
                                else
                                {
                                    pixData[pixDataStart + 0] = 0;
                                    pixData[pixDataStart + 1] = 0;
                                    pixData[pixDataStart + 2] = 0;
                                    pixData[pixDataStart + 3] = 0;
                                }
                                break;
                            }

                            code >>= 2;
                        }
                    }


                }
            }

            return std::move(pixData);
        }

        //buffer DecodeDXT3(buffer& data, int width, int height)
        //{
        //    buffer pixData(width*height * 4);

        //    int xBlocks = width / 4;
        //    int yBlocks = height / 4;
        //    for (int y = 0; y < yBlocks; y++)
        //    {
        //        for (int x = 0; x < xBlocks; x++)
        //        {
        //            int blockDataStart = ((y * xBlocks) + x) * 16;
        //            std::vector<u16> alphaData(4);

        //            alphaData[0] = (u16)((data[blockDataStart + 0] << 8) + data[blockDataStart + 1]);
        //            alphaData[1] = (u16)((data[blockDataStart + 2] << 8) + data[blockDataStart + 3]);
        //            alphaData[2] = (u16)((data[blockDataStart + 4] << 8) + data[blockDataStart + 5]);
        //            alphaData[3] = (u16)((data[blockDataStart + 6] << 8) + data[blockDataStart + 7]);

        //            //byte[, ] alpha = new byte[4, 4];
        //            //for (int j = 0; j < 4; j++)
        //            //{
        //            //    for (int i = 0; i < 4; i++)
        //            //    {
        //            //        alpha[i, j] = (byte)((alphaData[j] & 0xF) * 16);
        //            //        alphaData[j] >>= 4;
        //            //    }
        //            //}

        //            u16 color0 = (u16)((data[blockDataStart + 8] << 8) + data[blockDataStart + 9]);
        //            u16 color1 = (u16)((data[blockDataStart + 10] << 8) + data[blockDataStart + 11]);

        //            u32 code = *(u32*)(data.data() + (blockDataStart + 8 + 4));

        //            u16 r0 = 0, g0 = 0, b0 = 0, r1 = 0, g1 = 0, b1 = 0;
        //            r0 = (u16)(8 * (color0 & 31));
        //            g0 = (u16)(4 * ((color0 >> 5) & 63));
        //            b0 = (u16)(8 * ((color0 >> 11) & 31));

        //            r1 = (u16)(8 * (color1 & 31));
        //            g1 = (u16)(4 * ((color1 >> 5) & 63));
        //            b1 = (u16)(8 * ((color1 >> 11) & 31));

        //            for (int k = 0; k < 4; k++)
        //            {
        //                int j = k ^ 1;

        //                for (int i = 0; i < 4; i++)
        //                {
        //                    int pixDataStart = (width * (y * 4 + j) * 4) + ((x * 4 + i) * 4);
        //                    uint codeDec = code & 0x3;

        //                    pixData[pixDataStart + 3] = alpha[i, j];

        //                    switch (codeDec)
        //                    {
        //                    case 0:
        //                        pixData[pixDataStart + 0] = (byte)r0;
        //                        pixData[pixDataStart + 1] = (byte)g0;
        //                        pixData[pixDataStart + 2] = (byte)b0;
        //                        break;
        //                    case 1:
        //                        pixData[pixDataStart + 0] = (byte)r1;
        //                        pixData[pixDataStart + 1] = (byte)g1;
        //                        pixData[pixDataStart + 2] = (byte)b1;
        //                        break;
        //                    case 2:
        //                        if (color0 > color1)
        //                        {
        //                            pixData[pixDataStart + 0] = (byte)((2 * r0 + r1) / 3);
        //                            pixData[pixDataStart + 1] = (byte)((2 * g0 + g1) / 3);
        //                            pixData[pixDataStart + 2] = (byte)((2 * b0 + b1) / 3);
        //                        }
        //                        else
        //                        {
        //                            pixData[pixDataStart + 0] = (byte)((r0 + r1) / 2);
        //                            pixData[pixDataStart + 1] = (byte)((g0 + g1) / 2);
        //                            pixData[pixDataStart + 2] = (byte)((b0 + b1) / 2);
        //                        }
        //                        break;
        //                    case 3:
        //                        if (color0 > color1)
        //                        {
        //                            pixData[pixDataStart + 0] = (byte)((r0 + 2 * r1) / 3);
        //                            pixData[pixDataStart + 1] = (byte)((g0 + 2 * g1) / 3);
        //                            pixData[pixDataStart + 2] = (byte)((b0 + 2 * b1) / 3);
        //                        }
        //                        else
        //                        {
        //                            pixData[pixDataStart + 0] = 0;
        //                            pixData[pixDataStart + 1] = 0;
        //                            pixData[pixDataStart + 2] = 0;
        //                        }
        //                        break;
        //                    }

        //                    code >>= 2;
        //                }
        //            }


        //        }
        //    }
        //    return pixData;
        //}

        u32 ReadDXNBlockBits(buffer& data, int blockStart)
        {
            u32 blockBits = 0;

            blockBits |= data[blockStart + 6];
            blockBits <<= 8;
            blockBits |= data[blockStart + 7];
            blockBits <<= 8;
            blockBits |= data[blockStart + 4];
            blockBits <<= 8;
            blockBits |= data[blockStart + 5];
            blockBits <<= 8;
            blockBits |= data[blockStart + 2];
            blockBits <<= 8;
            blockBits |= data[blockStart + 3];

            return blockBits;
        }

        buffer DecodeDXN(buffer& data, int width, int height)
        {
            buffer pixData(width * height * 4);

            int xBlocks = width / 4;
            int yBlocks = height / 4;
            for (int y = 0; y < yBlocks; y++)
            {
                for (int x = 0; x < xBlocks; x++)
                {
                    int blockStart = ((y * xBlocks) + x) * 16;
                    std::array<u8, 8> red;
                    red[1] = data[blockStart];
                    red[0] = data[blockStart + 1];
                    if (red[0] > red[1])
                    {
                        red[2] = (u8)((6 * red[0] + 1 * red[1]) / 7);
                        red[3] = (u8)((5 * red[0] + 2 * red[1]) / 7);
                        red[4] = (u8)((4 * red[0] + 3 * red[1]) / 7);
                        red[5] = (u8)((3 * red[0] + 4 * red[1]) / 7);
                        red[6] = (u8)((2 * red[0] + 5 * red[1]) / 7);
                        red[7] = (u8)((1 * red[0] + 6 * red[1]) / 7);
                    }
                    else
                    {
                        red[2] = (u8)((4 * red[0] + 1 * red[1]) / 5);
                        red[3] = (u8)((3 * red[0] + 2 * red[1]) / 5);
                        red[4] = (u8)((2 * red[0] + 3 * red[1]) / 5);
                        red[5] = (u8)((1 * red[0] + 4 * red[1]) / 5);
                        red[6] = 0;
                        red[7] = 0xff;
                    }

                    u32 blockBits = ReadDXNBlockBits(data, blockStart);

                    std::array<u8, 16> redIndices;
                    for (int i = 0; i < 16; i++)
                    {
                        redIndices[i] = (u8)((blockBits >> (3 * i)) & 0x7);
                    }

                    blockStart += 8;

                    std::array<u8, 8> green;
                    green[1] = data[blockStart];
                    green[0] = data[blockStart + 1];

                    if (green[0] > green[1])
                    {
                        green[2] = (u8)((6 * green[0] + 1 * green[1]) / 7);
                        green[3] = (u8)((5 * green[0] + 2 * green[1]) / 7);
                        green[4] = (u8)((4 * green[0] + 3 * green[1]) / 7);
                        green[5] = (u8)((3 * green[0] + 4 * green[1]) / 7);
                        green[6] = (u8)((2 * green[0] + 5 * green[1]) / 7);
                        green[7] = (u8)((1 * green[0] + 6 * green[1]) / 7);
                    }
                    else
                    {
                        green[2] = (u8)((4 * green[0] + 1 * green[1]) / 5);
                        green[3] = (u8)((3 * green[0] + 2 * green[1]) / 5);
                        green[4] = (u8)((2 * green[0] + 3 * green[1]) / 5);
                        green[5] = (u8)((1 * green[0] + 4 * green[1]) / 5);
                        green[6] = 0;
                        green[7] = 0xff;
                    }

                    blockBits = 0;
                    blockBits = ReadDXNBlockBits(data, blockStart);

                    std::array<u8, 16> greenIndices;
                    for (int i = 0; i < 16; i++)
                    {
                        greenIndices[i] = (u8)((blockBits >> (i * 3)) & 0x7);
                    }


                    for (int pY = 0; pY < 4; pY++)
                    {
                        int j = pY;// ^ 1;
                        for (int pX = 0; pX < 4; pX++)
                        {
                            int pixDataStart = (width * (y * 4 + j) * 4) + ((x * 4 + pX) * 4);
                            int colID = pY * 4 + pX;
                            auto colRed = red[redIndices[colID]];
                            auto colBlue = green[greenIndices[colID]];
                            pixData[pixDataStart] = 0xff;
                            pixData[pixDataStart + 1] = colBlue;
                            pixData[pixDataStart + 2] = colRed;
                            pixData[pixDataStart + 3] = 0xff;
                        }
                    }
                }
            }

            return std::move(pixData);

        }

        //buffer DecodeDXT5(buffer& data, int width, int height)
        //{
        //    byte[] pixData = new byte[width * height * 4];
        //    int xBlocks = width / 4;
        //    int yBlocks = height / 4;
        //    for (int y = 0; y < yBlocks; y++)
        //    {
        //        for (int x = 0; x < xBlocks; x++)
        //        {
        //            int blockDataStart = ((y * xBlocks) + x) * 16;
        //            uint[] alphas = new uint[8];
        //            ulong alphaMask = 0;

        //            alphas[0] = data[blockDataStart + 1];
        //            alphas[1] = data[blockDataStart + 0];

        //            alphaMask |= data[blockDataStart + 6];
        //            alphaMask <<= 8;
        //            alphaMask |= data[blockDataStart + 7];
        //            alphaMask <<= 8;
        //            alphaMask |= data[blockDataStart + 4];
        //            alphaMask <<= 8;
        //            alphaMask |= data[blockDataStart + 5];
        //            alphaMask <<= 8;
        //            alphaMask |= data[blockDataStart + 2];
        //            alphaMask <<= 8;
        //            alphaMask |= data[blockDataStart + 3];


        //            // 8-alpha or 6-alpha block
        //            if (alphas[0] > alphas[1])
        //            {
        //                // 8-alpha block: derive the other 6
        //                // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
        //                alphas[2] = (byte)((6 * alphas[0] + 1 * alphas[1] + 3) / 7);    // bit code 010
        //                alphas[3] = (byte)((5 * alphas[0] + 2 * alphas[1] + 3) / 7);    // bit code 011
        //                alphas[4] = (byte)((4 * alphas[0] + 3 * alphas[1] + 3) / 7);    // bit code 100
        //                alphas[5] = (byte)((3 * alphas[0] + 4 * alphas[1] + 3) / 7);    // bit code 101
        //                alphas[6] = (byte)((2 * alphas[0] + 5 * alphas[1] + 3) / 7);    // bit code 110
        //                alphas[7] = (byte)((1 * alphas[0] + 6 * alphas[1] + 3) / 7);    // bit code 111
        //            }
        //            else
        //            {
        //                // 6-alpha block.
        //                // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
        //                alphas[2] = (byte)((4 * alphas[0] + 1 * alphas[1] + 2) / 5);    // Bit code 010
        //                alphas[3] = (byte)((3 * alphas[0] + 2 * alphas[1] + 2) / 5);    // Bit code 011
        //                alphas[4] = (byte)((2 * alphas[0] + 3 * alphas[1] + 2) / 5);    // Bit code 100
        //                alphas[5] = (byte)((1 * alphas[0] + 4 * alphas[1] + 2) / 5);    // Bit code 101
        //                alphas[6] = 0x00;                                               // Bit code 110
        //                alphas[7] = 0xFF;                                               // Bit code 111
        //            }

        //            byte[, ] alpha = new byte[4, 4];

        //            for (int i = 0; i < 4; i++)
        //            {
        //                for (int j = 0; j < 4; j++)
        //                {
        //                    alpha[j, i] = (byte)alphas[alphaMask & 7];
        //                    alphaMask >>= 3;
        //                }
        //            }

        //            ushort color0 = (ushort)((data[blockDataStart + 8] << 8) + data[blockDataStart + 9]);
        //            ushort color1 = (ushort)((data[blockDataStart + 10] << 8) + data[blockDataStart + 11]);

        //            uint code = BitConverter.ToUInt32(data, blockDataStart + 8 + 4);

        //            ushort r0 = 0, g0 = 0, b0 = 0, r1 = 0, g1 = 0, b1 = 0;
        //            r0 = (ushort)(8 * (color0 & 31));
        //            g0 = (ushort)(4 * ((color0 >> 5) & 63));
        //            b0 = (ushort)(8 * ((color0 >> 11) & 31));

        //            r1 = (ushort)(8 * (color1 & 31));
        //            g1 = (ushort)(4 * ((color1 >> 5) & 63));
        //            b1 = (ushort)(8 * ((color1 >> 11) & 31));

        //            for (int k = 0; k < 4; k++)
        //            {
        //                int j = k ^ 1;

        //                for (int i = 0; i < 4; i++)
        //                {
        //                    int pixDataStart = (width * (y * 4 + j) * 4) + ((x * 4 + i) * 4);
        //                    uint codeDec = code & 0x3;

        //                    pixData[pixDataStart + 3] = alpha[i, j];

        //                    switch (codeDec)
        //                    {
        //                    case 0:
        //                        pixData[pixDataStart + 0] = (byte)r0;
        //                        pixData[pixDataStart + 1] = (byte)g0;
        //                        pixData[pixDataStart + 2] = (byte)b0;
        //                        break;
        //                    case 1:
        //                        pixData[pixDataStart + 0] = (byte)r1;
        //                        pixData[pixDataStart + 1] = (byte)g1;
        //                        pixData[pixDataStart + 2] = (byte)b1;
        //                        break;
        //                    case 2:
        //                        if (color0 > color1)
        //                        {
        //                            pixData[pixDataStart + 0] = (byte)((2 * r0 + r1) / 3);
        //                            pixData[pixDataStart + 1] = (byte)((2 * g0 + g1) / 3);
        //                            pixData[pixDataStart + 2] = (byte)((2 * b0 + b1) / 3);
        //                        }
        //                        else
        //                        {
        //                            pixData[pixDataStart + 0] = (byte)((r0 + r1) / 2);
        //                            pixData[pixDataStart + 1] = (byte)((g0 + g1) / 2);
        //                            pixData[pixDataStart + 2] = (byte)((b0 + b1) / 2);
        //                        }
        //                        break;
        //                    case 3:
        //                        if (color0 > color1)
        //                        {
        //                            pixData[pixDataStart + 0] = (byte)((r0 + 2 * r1) / 3);
        //                            pixData[pixDataStart + 1] = (byte)((g0 + 2 * g1) / 3);
        //                            pixData[pixDataStart + 2] = (byte)((b0 + 2 * b1) / 3);
        //                        }
        //                        else
        //                        {
        //                            pixData[pixDataStart + 0] = 0;
        //                            pixData[pixDataStart + 1] = 0;
        //                            pixData[pixDataStart + 2] = 0;
        //                        }
        //                        break;
        //                    }

        //                    code >>= 2;
        //                }
        //            }


        //        }
        //    }
        //    return pixData;
        //}


        // https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/texture_conversion.cc


        void CopySwapBlock(void* output, const void* input, size_t length)
        {
            //switch (endian) {
            //case Endian::k8in16:
            //    xe::copy_and_swap_16_unaligned(output, input, length / 2);
            //    break;
            //case Endian::k8in32:
            //    xe::copy_and_swap_32_unaligned(output, input, length / 4);
            //    break;
            //case Endian::k16in32:  // Swap high and low 16 bits within a 32 bit word
            //    xe::copy_and_swap_16_in_32_unaligned(output, input, length);
            //    break;
            //default:
            //case Endian::kUnspecified:
            //    std::memcpy(output, input, length);
            //    break;
            //}
        }

        // https://fileadmin.cs.lth.se/cs/Personal/Michael_Doggett/talks/unc-xenos-doggett.pdf
        // (R is in the higher bits, according to how this format is used in Halo 3).

        void ConvertTexelCTX1ToR8G8(void* output, const void* input, size_t length)
        {
            union {
                u8 data[8];
                struct {
                    u8 g0, r0, g1, r1;
                    u32 xx;
                };
            } block;
            static_assert(sizeof(block) == 8, "CTX1 block mismatch");

            const uint32_t bytes_per_block = 8;
            CopySwapBlock(block.data, input, bytes_per_block);

            u8 cr[4] = {
                block.r0, block.r1,
                static_cast<u8>(2.f / 3.f * block.r0 + 1.f / 3.f * block.r1),
                static_cast<u8>(1.f / 3.f * block.r0 + 2.f / 3.f * block.r1) };
            u8 cg[4] = {
                block.g0, block.g1,
                static_cast<u8>(2.f / 3.f * block.g0 + 1.f / 3.f * block.g1),
                static_cast<u8>(1.f / 3.f * block.g0 + 2.f / 3.f * block.g1) };

            auto output_bytes = static_cast<uint8_t*>(output);
            for (uint32_t oy = 0; oy < 4; ++oy) {
                for (uint32_t ox = 0; ox < 4; ++ox) {
                    uint8_t xx = (block.xx >> (((ox + (oy * 4)) * 2))) & 3;
                    output_bytes[(oy * length) + (ox * 2) + 0] = cr[xx];
                    output_bytes[(oy * length) + (ox * 2) + 1] = cg[xx];
                }
            }
        }

        struct RGBAColor
        {
            u32 R, G, B, A;
        };

        RGBAColor GradientColors(const RGBAColor& Color1, const RGBAColor& Color2)
        {
            RGBAColor newColor;
            newColor.R = (u8)(((Color1.R * 2 + Color2.R)) / 3);
            newColor.G = (u8)(((Color1.G * 2 + Color2.G)) / 3);
            newColor.B = (u8)(((Color1.B * 2 + Color2.B)) / 3);
            newColor.A = 0xFF;
            return newColor;
        }

        RGBAColor GradientColorsHalf(const RGBAColor&Color1, const RGBAColor&Color2)
        {
            RGBAColor newColor;
            newColor.R = (u8)(Color1.R / 2 + Color2.R / 2);
            newColor.G = (u8)(Color1.G / 2 + Color2.G / 2);
            newColor.B = (u8)(Color1.B / 2 + Color2.B / 2);
            newColor.A = 0xFF;
            return newColor;
        }

        buffer DecodeCTX1(const buffer& data, int width, int height)
        {
            buffer DestData(width * height * 4);

            int dptr = 0;
            for (int i = 0; i < width * height; i += 16)
            {
                int c1 = data[dptr + 1] << 8 | data[dptr];
                int c2 = data[dptr + 3] << 8 | data[dptr + 2];

                std::array<RGBAColor, 4> colorTable;

                colorTable[0].R = data[dptr];
                colorTable[0].G = data[dptr + 1];
                colorTable[1].R = data[dptr + 2];
                colorTable[1].G = data[dptr + 3];

                if (c1 > c2)
                {
                    colorTable[2] = GradientColors(colorTable[0], colorTable[1]);
                    colorTable[3] = GradientColors(colorTable[1], colorTable[0]);
                }
                else
                {
                    colorTable[2] = GradientColorsHalf(colorTable[0], colorTable[1]);
                    colorTable[3] = colorTable[0];
                }

                int CData = (data[dptr + 5]) |
                    (data[dptr + 4] << 8) |
                    (data[dptr + 7] << 16) |
                    (data[dptr + 6] << 24);

                int ChunkNum = i / 16;
                int XPos = ChunkNum % (width / 4);
                int YPos = (ChunkNum - XPos) / (width / 4);

                int sizeh = (height < 4) ? height : 4;
                int sizew = (width < 4) ? width : 4;

                for (int x = 0; x < sizeh; x++)
                    for (int y = 0; y < sizew; y++)
                    {
                        RGBAColor CColor = colorTable[CData & 3];
                        CData >>= 2;

                        int tmp1 = ((YPos * 4 + x) * width + XPos * 4 + y) * 4;

                        float cx = ((CColor.R / 255.0f) * 2.0f) - 1.0f;
                        float cy = ((CColor.G / 255.0f) * 2.0f) - 1.0f;

                        float cz = std::sqrtf(std::max(0.0f, std::min(1.0f, 1.0f - cx * cx - cy * cy)));

                        DestData[tmp1] = (u8)(((cz + 1.0f) / 2.0f) * 255.0f);//(255 - (Math.Abs(CColor.R - CColor.G)));
                        DestData[tmp1 + 1] = (u8)CColor.G;
                        DestData[tmp1 + 2] = (u8)CColor.R;
                        DestData[tmp1 + 3] = 255;
                    }
                dptr += 8;
            }

            return DestData;
        }
    }
}
