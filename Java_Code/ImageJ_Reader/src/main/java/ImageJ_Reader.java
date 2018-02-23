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
        String dir = args[0];
        String name = args[1];
        String id = dir + name;
        ImageProcessorReader ip_reader = new ImageProcessorReader(
                new ChannelSeparator(LociPrefs.makeImageReader()));
        try {
            IJ.showStatus("Examining file " + name);
            ip_reader.setId(id);
            int num = ip_reader.getImageCount();
            int width = ip_reader.getSizeX();
            int height = ip_reader.getSizeY();
            ImageStack stack = new ImageStack(width, height);
            byte[][][] lookupTable = new byte[ip_reader.getSizeC()][][];

            //TODO: Don't know how to handle multiple channels i.e RGb images currently.
            // Adding all the slices into a 2D array and returning those values.
            int[][][] return_array = new int[height][width][num];
            for (int i=0; i<num; i++) {
                IJ.showStatus("Reading image plane #" + (i + 1) + "/" + num);
                ImageProcessor ip = ip_reader.openProcessors(i)[0];

                // Copying the value to the return array.
                int[][] temp_array = ip.getIntArray();
                for (int h=0; h < height; h++) {
                    for (int w = 0; w < width; w++) {
                        return_array[h][w][i] = temp_array[h][w];
                    }
                }

                java.awt.image.BufferedImage awt_Ip =  ip.getBufferedImage();
                ImageIO.write(awt_Ip, "jpg", new File("D:\\Dev_Environment\\Test_Files\\Test_Folder\\out" + Integer.toString(i) + ".jpg"));

                stack.addSlice("" + (i + 1), ip);
                int channel = ip_reader.getZCTCoords(i)[1];
                lookupTable[channel] = ip_reader.get8BitLookupTable();
            }
            IJ.showStatus("Constructing image");
            ImagePlus imp = new ImagePlus(name, stack);

            // ImagePlus show is leading to java window not responding, maybe it is because this program is not a imageJ plugin but a standalone program.
            //imp.show();

            //ImagePlus colorizedImage = applyLookupTables(r, imp, lookupTable);
            //r.close();
            //colorizedImage.show();
            IJ.showStatus("");
        }
        catch (FormatException exc) {
            IJ.error("Sorry, an error occurred: " + exc.getMessage());
        }
        catch (IOException exc) {
            IJ.error("Sorry, an error occurred: " + exc.getMessage());
        }
    }

    private static ImagePlus applyLookupTables(IFormatReader r, ImagePlus imp, byte[][][] lookupTable)
    {
        // apply color lookup tables, if present
        // this requires ImageJ v1.39 or higher
        if (r.isIndexed()) {
            CompositeImage composite =
                    new CompositeImage(imp, CompositeImage.COLOR);
            for (int c=0; c<r.getSizeC(); c++) {
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
    public static int getDepth(String[] args) {
        String dir = args[0];
        String name = args[1];
        String id = dir + name;
        ImageProcessorReader ip_reader = new ImageProcessorReader( new ChannelSeparator(LociPrefs.makeImageReader()));
        try {
            IJ.showStatus("Examining file " + name);
            ip_reader.setId(id);
            int num = ip_reader.getImageCount();
            return num;
        }
        catch (FormatException exc) {
            IJ.error("Sorry, an error occurred: " + exc.getMessage());
        }
        catch (IOException exc) {
            IJ.error("Sorry, an error occurred: " + exc.getMessage());
        }
        return 0;
    }

    public static int[][][] getIntData(String[] args) {
        String dir = args[0];
        String name = args[1];
        String id = dir + name;
        ImageProcessorReader ip_reader = new ImageProcessorReader(
                new ChannelSeparator(LociPrefs.makeImageReader()));
        try {
            IJ.showStatus("Examining file " + name);
            ip_reader.setId(id);
            int num = ip_reader.getImageCount();
            int width = ip_reader.getSizeX();
            int height = ip_reader.getSizeY();
            ImageStack stack = new ImageStack(width, height);
            byte[][][] lookupTable = new byte[ip_reader.getSizeC()][][];

            //TODO: Don't know how to handle multiple channels i.e RGb images currently.
            // Adding all the slices into a 2D array and returning those values.
            int[][][] return_array = new int[height][width][num];
            for (int i=0; i<num; i++) {
                IJ.showStatus("Reading image plane #" + (i + 1) + "/" + num);
                ImageProcessor ip = ip_reader.openProcessors(i)[0];

                // Copying the value to the return array.
                int[][] temp_array = ip.getIntArray();
                for (int h=0; h < height; h++) {
                    for (int w = 0; w < width; w++) {
                        return_array[h][w][i] = temp_array[h][w];
                    }
                }

                //java.awt.image.BufferedImage awt_Ip =  ip.getBufferedImage();
                //ImageIO.write(awt_Ip, "jpg", new File("D:\\Dev_Environment\\Test_Files\\Test_Folder\\out" + Integer.toString(i) + ".jpg"));

                stack.addSlice("" + (i + 1), ip);
                int channel = ip_reader.getZCTCoords(i)[1];
                lookupTable[channel] = ip_reader.get8BitLookupTable();
            }
            IJ.showStatus("Constructing image");
            ImagePlus imp = new ImagePlus(name, stack);

            // ImagePlus show is leading to java window not responding, maybe it is because this program is not a imageJ plugin but a standalone program.
            //imp.show();

            //ImagePlus colorizedImage = applyLookupTables(r, imp, lookupTable);
            //r.close();
            //colorizedImage.show();
            IJ.showStatus("");
            return return_array;
        }
        catch (FormatException exc) {
            IJ.error("Sorry, an error occurred: " + exc.getMessage());
        }
        catch (IOException exc) {
            IJ.error("Sorry, an error occurred: " + exc.getMessage());
        }
        return null;
    }
}
