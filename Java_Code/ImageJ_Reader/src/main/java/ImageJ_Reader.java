import ij.process.ImageProcessor;
import ij.process.LUT;
import loci.formats.ChannelSeparator;
import loci.formats.IFormatReader;
import ij.ImagePlus;
import ij.ImageStack;
import ij.CompositeImage;

import java.io.File;
import java.io.IOException;

import ij.IJ;

import javax.imageio.ImageIO;

import loci.formats.FormatException;
import loci.plugins.util.ImageProcessorReader;
import loci.plugins.util.LociPrefs;

//TODO: Add threads to load the images fast.
public class ImageJ_Reader {
    public static void main(String[] args) {
        System.out.println("Yes!!!");
        int[] test = getIntDataB(args, 0, 2);
        int[] test2 = getMetaData(args);
        byte[] test3 = getByteData(args, 0, 2);
    }

    private static ImagePlus applyLookupTables(IFormatReader r, ImagePlus imp, byte[][][] lookupTable) {
        // apply color lookup tables, if present
        // this requires ImageJ v1.39 or higher
        if (r.isIndexed()) {
            CompositeImage composite =
                    new CompositeImage(imp, CompositeImage.COLOR);
            for (int c = 0; c < r.getSizeC(); c++) {
                composite.setPosition(c + 1, 1, 1);
                LUT lut = new LUT(lookupTable[c][0], lookupTable[c][1], lookupTable[c][2]);
                composite.setChannelLut(lut);
            }
            composite.setPosition(1, 1, 1);
            return composite;
        }
        return imp;
    }

    //The method for returning number of z channel images.
    public static int[] getMetaData(String[] args) {
        int[] dump = new int[1];
        dump[0] = -1;
        String id = args[0];
        ImageProcessorReader ip_reader = new ImageProcessorReader(new ChannelSeparator(LociPrefs.makeImageReader()));
        try {
            //IJ.showStatus("Examining file " + name);
            ip_reader.setId(id);

            int[] metadata = new int[7];
            metadata[0] = ip_reader.getImageCount();
            metadata[1] = ip_reader.getSizeX();
            metadata[2] = ip_reader.getSizeY();
            metadata[3] = ip_reader.getSizeZ();
            metadata[4] = ip_reader.getSizeC();
            metadata[5] = ip_reader.getSizeT();
            metadata[6] = ip_reader.getBitsPerPixel();
            //int test = ip_reader.getPixelType();
            return metadata;
        } catch (FormatException exc) {
            dump[0] = -2;
        } catch (IOException exc) {
            dump[0] = -3;
        }
        return dump;
    }

    public static int[] getIntDataS(String[] args) {
        int[] test = new int[10];
        test[0] = 10;
        test[1] = 10;
        test[2] = 10;
        test[3] = 10;
        test[4] = 10;
        test[5] = 10;
        test[6] = 10;
        test[7] = 10;
        test[8] = 10;
        test[9] = 10;
        return test;
    }

    public static int[] getIntDataB(String[] args, int time_id, int channel_id) {
        String id = args[0];
        ImageProcessorReader ip_reader = new ImageProcessorReader(new ChannelSeparator(LociPrefs.makeImageReader()));

        try {
            ip_reader.setId(id);
            int num = ip_reader.getImageCount();
            int width = ip_reader.getSizeX();
            int height = ip_reader.getSizeY();
            int channels = ip_reader.getSizeC();
            int depth = ip_reader.getSizeZ();
            int time_seq = ip_reader.getSizeT();

            ImageStack stack = new ImageStack(width, height);
            byte[][][] lookupTable = new byte[ip_reader.getSizeC()][][];

            //TODO: Don't know how to handle multiple channels i.e RGb images currently.
            // Adding all the slices into a 2D array and returning those values.
            int time_step = channels * depth;
            //int offset = time_id * time_step + depth_id * channels;
            int time_offset = time_id * time_step;

            int[] test_array = new int[width * height * depth];

            //TODO: Convert to byte.
            int pixel_offset = 0;
            for (int d = 0; d < depth; ++d){
                pixel_offset = d * width * height;
                int depth_offset = time_offset + (d * channels);

                ImageProcessor ip = ip_reader.openProcessors(depth_offset+channel_id)[0];
                int[][] temp_array = ip.getIntArray();
                for (int h = 0; h < height; h++) {
                    for (int w = 0; w < width; w++) {
                        test_array[pixel_offset + (h * width) + w] = temp_array[h][w];
                    }
                }
                //java.awt.image.BufferedImage awt_Ip = ip.getBufferedImage();
                //ImageIO.write(awt_Ip, "jpg", new File("D:\\Dev_Environment\\Test_Files\\Test_Folder\\outNB" + Integer.toString(depth_offset) + ".jpg"));
            }
            return test_array;
        } catch (FormatException exc) {
            int[] test = new int[1];
            test[0] = 1;
            return test;
            //IJ.error("Sorry, an error occurred: " + exc.getMessage());
        } catch (IOException exc) {
            int[] test = new int[1];
            test[0] = 2;
            return test;
            //IJ.error("Sorry, an error occurred: " + exc.getMessage());
        }
    }

    public static byte[] getByteData(String[] args, int time_id, int channel_id) {
        String id = args[0];
        ImageProcessorReader ip_reader = new ImageProcessorReader(new ChannelSeparator(LociPrefs.makeImageReader()));

        try {
            ip_reader.setId(id);
            int num = ip_reader.getImageCount();
            int width = ip_reader.getSizeX();
            int height = ip_reader.getSizeY();
            int channels = ip_reader.getSizeC();
            int depth = ip_reader.getSizeZ();
            int time_seq = ip_reader.getSizeT();

            ImageStack stack = new ImageStack(width, height);
            byte[][][] lookupTable = new byte[ip_reader.getSizeC()][][];

            // Adding all the slices into a 2D array and returning those values.
            int time_step = channels * depth;
            //int offset = time_id * time_step + depth_id * channels;
            int time_offset = time_id * time_step;

            byte[] test_array = new byte[width * height * depth];
            int pixel_offset = 0;
            for (int d = 0; d < depth; ++d){
                pixel_offset = d * width * height;
                int depth_offset = time_offset + (d * channels);

                ImageProcessor ip = ip_reader.openProcessors(depth_offset+channel_id)[0];
                int[][] temp_array = ip.getIntArray();
                for (int h = 0; h < height; h++) {
                    for (int w = 0; w < width; w++) {
                        test_array[pixel_offset + (h * width) + w] = (byte)temp_array[h][w];
                    }
                }
                //java.awt.image.BufferedImage awt_Ip = ip.getBufferedImage();
                //ImageIO.write(awt_Ip, "jpg", new File("D:\\Dev_Environment\\Test_Files\\Test_Folder\\outNB" + Integer.toString(depth_offset) + ".jpg"));
            }
            return test_array;
        } catch (FormatException exc) {
            byte[] test = new byte[1];
            test[0] = 1;
            return test;
        } catch (IOException exc) {
            byte[] test = new byte[1];
            test[0] = 2;
            return test;
        }
    }
}
