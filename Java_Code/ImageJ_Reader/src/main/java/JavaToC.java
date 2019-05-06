import java.nio.ByteBuffer;

public class JavaToC {
    static{
        System.loadLibrary("JavaToC");
    }

    //Later reuse the bytebuffer as doublebytebuffer and use GetDirectBufferAddress to pass on to C++.
    public native ByteBuffer allocateMemory(int capacity);
    public native void writeToMemory(ByteBuffer buffer);

    /* Stub to give calls.
    public static void main(String[] args) {
        ByteBuffer memory = new JavaToC().allocateMemory(1024);
        System.out.println(memory.capacity() + "works");
    }
    */
}
