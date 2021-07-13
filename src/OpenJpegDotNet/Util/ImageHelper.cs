using System;
using System.Drawing;
using System.Drawing.Imaging;

// ReSharper disable once CheckNamespace
namespace OpenJpegDotNet
{

    internal static class ImageHelper
    {

        #region Methods

        public static Image FromRaw(byte[] raw, int width, int height, int stride, int channels, bool interleaved, bool lossless = true)
        {
            if (raw == null)
                throw new ArgumentNullException(nameof(raw));
            
            var byteAllocated = 1;
            var colorSpace = ColorSpace.Srgb;
            var precision = 24u / (uint)channels;
            var gap = stride - width * channels;

            using (var compressionParameters = new CompressionParameters())
            {
                OpenJpeg.SetDefaultEncoderParameters(compressionParameters);

                var subsamplingDx = compressionParameters.SubsamplingDx;
                var subsamplingDy = compressionParameters.SubsamplingDy;

                var componentParametersArray = new ImageComponentParameters[channels];
                for (var i = 0; i < channels; i++)
                {
                    componentParametersArray[i] = new ImageComponentParameters
                    {
                        Precision = precision,
                        Bpp = precision,
                        Signed = false,
                        Dx = (uint)subsamplingDx,
                        Dy = (uint)subsamplingDy,
                        Width = (uint)width,
                        Height = (uint)height
                    };
                }

                if (lossless) {
                    compressionParameters.TcpNumLayers = 1;
                    float[] rates = { 0 };
                    compressionParameters.TcpRates = rates;
                } else {
                    compressionParameters.TcpNumLayers = 5;
                    float[] rates = { 1920, 480, 120, 30, 10, 1 };
                    compressionParameters.TcpRates = rates;
                    if (channels >= 3) { compressionParameters.TcpMCT = 1; }
                }

                var image = OpenJpeg.ImageCreate((uint)channels, componentParametersArray, colorSpace);
                if (image == null)
                    return null;
                
                image.X0 = (uint)compressionParameters.ImageOffsetX0;
                image.Y0 = (uint)compressionParameters.ImageOffsetY0;
                image.X1 = image.X0 == 0 ? (uint)(width - 1) * (uint)subsamplingDx + 1 : image.X0 + (uint)(width - 1) * (uint)subsamplingDx + 1;
                image.Y1 = image.Y0 == 0 ? (uint)(height - 1) * (uint)subsamplingDy + 1 : image.Y0 + (uint)(height - 1) * (uint)subsamplingDy + 1;

                unsafe
                {
                    fixed (byte* pRaw = &raw[0])
                    {
                        // Bitmap data is interleave.
                        // Convert it to planer
                        if (byteAllocated == 1)
                        {
                            if (interleaved)
                            {
                                for (var i = 0; i < channels; i++)
                                {
                                    var target = image.Components[i].Data;
                                    var pTarget = (int*)target;
                                    var source = pRaw + i;
                                    for (var y = 0; y < height; y++)
                                    {
                                        for (var x = 0; x < width; x++)
                                        {
                                            *pTarget = *source;
                                            pTarget++;
                                            source += channels;
                                        }

                                        source += gap;
                                    }
                                }
                            }
                            else
                            {
                                for (var i = 0; i < channels; i++)
                                {
                                    var target = image.Components[i].Data;
                                    var pTarget = (int*)target;
                                    var source = pRaw + i * (stride * height);
                                    for (var y = 0; y < height; y++)
                                    {
                                        for (var x = 0; x < width; x++)
                                        {
                                            *pTarget = *source;
                                            pTarget++;
                                            source++;
                                        }

                                        source += gap;
                                    }
                                }
                            }
                        }
                    }
                }

                return image;
            }
        }

        public static Image FromBitmap(Bitmap bitmap, bool lossless = true)
        {
            if (bitmap == null)
                throw new ArgumentNullException(nameof(bitmap));

            var width = bitmap.Width;
            var height = bitmap.Height;
            var format = bitmap.PixelFormat;
            int channels;
            var byteAllocated = 0;
            ColorSpace colorSpace;
            switch (format)
            {
                case PixelFormat.Format24bppRgb:
                    channels = 3;
                    colorSpace = ColorSpace.Srgb;
                    byteAllocated = 1;
                    break;
                case PixelFormat.Format32bppArgb:
                    channels = 4;
                    colorSpace = ColorSpace.Srgb;
                    byteAllocated = 1;
                    break;
                case PixelFormat.Format8bppIndexed:
                    channels = 1;
                    colorSpace = ColorSpace.Srgb;
                    byteAllocated = 1;
                    break;
                default:
                    throw new NotSupportedException();
            }
            var precision = 24u / (uint)channels;

            BitmapData bitmapData = null;

            try
            {
                bitmapData = bitmap.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.ReadOnly, format);
                var stride = bitmapData.Stride;
                var gap = stride - width * channels;
                var scan0 = bitmapData.Scan0;

                using (var compressionParameters = new CompressionParameters())
                {
                    OpenJpeg.SetDefaultEncoderParameters(compressionParameters);

                    var subsamplingDx = compressionParameters.SubsamplingDx;
                    var subsamplingDy = compressionParameters.SubsamplingDy;

                    var componentParametersArray = new ImageComponentParameters[channels];
                    for (var i = 0; i < channels; i++)
                    {
                        componentParametersArray[i] = new ImageComponentParameters
                        {
                            Precision = precision,
                            Bpp = precision,
                            Signed = false,
                            Dx = (uint)subsamplingDx,
                            Dy = (uint)subsamplingDy,
                            Width = (uint)width,
                            Height = (uint)height
                        };
                    }

                    if (lossless) {
                        compressionParameters.TcpNumLayers = 1;
                        float[] rates = { 0 };
                        compressionParameters.TcpRates = rates;
                    } else {
                        compressionParameters.TcpNumLayers = 5;
                        float[] rates = { 1920, 480, 120, 30, 10, 1 };
                        compressionParameters.TcpRates = rates;
                        if (channels >= 3) { compressionParameters.TcpMCT = 1; }
                    }

                    var image = OpenJpeg.ImageCreate((uint)channels, componentParametersArray, colorSpace);
                    if (image == null)
                        return null;

                    image.X0 = (uint)compressionParameters.ImageOffsetX0;
                    image.Y0 = (uint)compressionParameters.ImageOffsetY0;
                    image.X1 = image.X0 == 0 ? (uint)(width - 1) * (uint)subsamplingDx + 1 : image.X0 + (uint)(width - 1) * (uint)subsamplingDx + 1;
                    image.Y1 = image.Y0 == 0 ? (uint)(height - 1) * (uint)subsamplingDy + 1 : image.Y0 + (uint)(height - 1) * (uint)subsamplingDy + 1;

                    unsafe
                    {
                        // Bitmap data is interleave.
                        // Convert it to planer
                        if (byteAllocated == 1)
                        {
                            for (var i = 0; i < channels; i++)
                            {
                                var target = image.Components[i].Data;
                                var pTarget = (int*)target;
                                var source = (byte*)scan0;
                                source += i;
                                for (var y = 0; y < height; y++)
                                {
                                    for (var x = 0; x < width; x++)
                                    {
                                        *pTarget = *source;
                                        pTarget++;
                                        source += channels;
                                    }

                                    source += gap;
                                }
                            }
                        }
                    }

                    return image;
                }
            }
            finally
            {
                if (bitmapData != null)
                    bitmap.UnlockBits(bitmapData);
            }
        }

        #endregion

    }

}
