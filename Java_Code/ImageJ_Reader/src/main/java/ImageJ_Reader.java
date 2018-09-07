import ij.process.ImageProcessor;
import ij.process.LUT;
import loci.common.services.ServiceFactory;
import loci.formats.ChannelSeparator;
import loci.formats.IFormatReader;
import ij.ImagePlus;
import ij.ImageStack;
import ij.CompositeImage;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import ij.IJ;

import javax.imageio.ImageIO;

import loci.formats.FormatException;
import loci.formats.ImageReader;
import loci.formats.meta.IMetadata;
import loci.formats.services.OMEXMLService;
import loci.plugins.util.ImageProcessorReader;
import loci.plugins.util.LociPrefs;
import ome.units.quantity.Length;
import ome.units.quantity.Time;

//TODO: Add threads to load the images fast.
public class ImageJ_Reader {
    public static void main(String[] args) {
        short[] test = getIntDataB(args, 0, 1);
        int[] test2 = getMetaData(args);
        //byte[] test3 = getByteData(args, 0, 0);
        //byte[] test3 = getIntDataB(args, 0, 0);
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
/*
    public static int[] getMetaDataN(String[] args) {
        String id = args[0];
        //ImageProcessorReader ip_reader = new ImageProcessorReader(new ChannelSeparator(LociPrefs.makeImageReader()));
        ImageReader ip_reader = new ImageReader();
        try {
            ip_reader.setId(id);

            int[] metadata = new int[7];
            metadata[0] = ip_reader.getImageCount();
            metadata[1] = ip_reader.getSizeX();
            metadata[2] = ip_reader.getSizeY();
            metadata[3] = ip_reader.getSizeZ();
            metadata[4] = ip_reader.getSizeC();
            metadata[5] = ip_reader.getSizeT();
            metadata[6] = ip_reader.getPixelType();

            long totalsize = (long)metadata[1] * (long)metadata[2] * (long)metadata[3];
            if (totalsize > Integer.MAX_VALUE){
                throw new Exception("Array size too big");
            }
            //int test = ip_reader.getPixelType();
            return metadata;
        } catch (FormatException exc) {
            int[] test = new int[1];
            test[0] = 4;
            return test;
        } catch (IOException exc) {
            int[] test = new int[1];
            test[0] = 5;
            return test;
        } catch (Exception exc){
            int[] test = new int[1];
            test[0] = 6;
            System.out.println(exc.getMessage());
            if (exc.getMessage() == "Array size too big"){
                test[0] = 7;
            }
            return test;
        }
    }
*/

    public static int[] getMetaData(String[] args) {
        String id = args[0];
        ImageProcessorReader ip_reader = new ImageProcessorReader(new ChannelSeparator(LociPrefs.makeImageReader()));
        try {
            // Setting up xml reading.
            ServiceFactory factory = new ServiceFactory();
            OMEXMLService service = factory.getInstance(OMEXMLService.class);
            IMetadata meta = service.createOMEXMLMetadata();
            ip_reader.setMetadataStore(meta);

            ip_reader.setId(id);

            // Reading the pixel level matadata.
            int[] metadata = new int[11];
            metadata[0] = ip_reader.getImageCount();
            metadata[1] = ip_reader.getSizeX();
            metadata[2] = ip_reader.getSizeY();
            metadata[3] = ip_reader.getSizeZ();
            metadata[4] = ip_reader.getSizeC();
            metadata[5] = ip_reader.getSizeT();
            metadata[6] = ip_reader.getPixelType();

            long totalsize = (long)metadata[1] * (long)metadata[2] * (long)metadata[3];
            if (totalsize > Integer.MAX_VALUE){
                throw new Exception("Array size too big");
            }

            // Reading the physical metadata. TODO: I dont know what this 0 is for ? :(. Documentation is not helpful.
            Length physicalSizeX = meta.getPixelsPhysicalSizeX(0);
            Length physicalSizeY = meta.getPixelsPhysicalSizeY(0);
            Length physicalSizeZ = meta.getPixelsPhysicalSizeZ(0);
            Time timeIncrement = meta.getPixelsTimeIncrement(0);

            int px = (int)(physicalSizeX.value().doubleValue() * 10000);
            int py = (int)(physicalSizeY.value().doubleValue() * 10000);
            int pz = (int)(physicalSizeZ.value().doubleValue() * 10000);
            int pt = (int)(timeIncrement.value().doubleValue() * 10000);

            metadata[7] = px;
            metadata[8] = py;
            metadata[9] = pz;
            metadata[10] = pt;

            return metadata;
        } catch (FormatException exc) {
            int[] test = new int[1];
            test[0] = 4;
            return test;
        } catch (IOException exc) {
            int[] test = new int[1];
            test[0] = 5;
            return test;
        } catch (Exception exc){
            int[] test = new int[1];
            test[0] = 6;
            System.out.println(exc.getMessage());
            if (exc.getMessage() == "Array size too big"){
                test[0] = 7;
            }
            return test;
        }
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

/*
    public static byte[] getIntDataB(String[] args, int time_id, int channel_id) {
        String id = args[0];
        //ImageProcessorReader ip_reader = new ImageProcessorReader(new ChannelSeparator(LociPrefs.makeImageReader()));
        ImageReader ip_reader = new ImageReader();
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

                byte[] slice = ip_reader.openBytes(d); //A slice contains n channels.
                for (int h = 0; h < height; h++) {
                    for (int w = 0; w < width; w++) {
                        test_array[pixel_offset + (h * width) + w] = slice[(channel_id*height*width) + (h*width) + w];
                    }
                }

//                ImageProcessor ip = ip_reader.openProcessors(depth_offset+channel_id)[0];
//                int[][] temp_array = ip.getIntArray();
//                for (int h = 0; h < height; h++) {
//                    for (int w = 0; w < width; w++) {
//                        test_array[pixel_offset + (h * width) + w] = (short)temp_array[w][h];
//                    }
//                }
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
        } catch (Exception exc){
            byte[] test = new byte[1];
            test[0] = 3;
            return test;
        }
    }
*/


    public static short[] getIntDataB(String[] args, int time_id, int channel_id) {
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

            short[] test_array = new short[width * height * depth];

            int pixel_offset = 0;
            for (int d = 0; d < depth; ++d){
                pixel_offset = d * width * height;
                int depth_offset = time_offset + (d * channels);

                ImageProcessor ip = ip_reader.openProcessors(depth_offset+channel_id)[0];
                int[][] temp_array = ip.getIntArray();
                for (int h = 0; h < height; h++) {
                    for (int w = 0; w < width; w++) {
                        test_array[pixel_offset + (h * width) + w] = (short)temp_array[w][h];
                    }
                }
                //java.awt.image.BufferedImage awt_Ip = ip.getBufferedImage();
                //ImageIO.write(awt_Ip, "jpg", new File("D:\\Dev_Environment\\Test_Files\\Test_Folder\\outNB" + Integer.toString(depth_offset) + ".jpg"));
            }
            return test_array;
        } catch (FormatException exc) {
            short[] test = new short[1];
            test[0] = 1;
            return test;
        } catch (IOException exc) {
            short[] test = new short[1];
            test[0] = 2;
            return test;
        } catch (Exception exc){
            short[] test = new short[1];
            test[0] = 3;
            return test;
        }
    }

    public static byte[] getByteData(String[] args, int time_id, int channel_id) {
        String id = args[0];
        //String id = "E:\\DATA\\Chinchun\\7.lsm";
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
                        test_array[pixel_offset + (h * width) + w] = (byte)temp_array[w][h];
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
        } catch (Exception exc) {
            byte[] test = new byte[1];
            test[0] = 3;
            return test;
        }
    }
}
