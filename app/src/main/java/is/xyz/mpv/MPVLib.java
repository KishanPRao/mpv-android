package is.xyz.mpv;

// Wrapper for native library

public class MPVLib {

     static {
        String[] libs = { "mpv", "player" };
        for (String lib: libs) {
            System.loadLibrary(lib);
        }
     }

     public static native void init();
     public static native void loadfile(String path);
     public static native void resize(int width, int height);
     public static native void step();
     public static native void play();
     public static native void pause();
     public static native void touch_down(int x, int y);
     public static native void touch_move(int x, int y);
     public static native void touch_up(int x, int y);
     public static native void setconfigdir(String path);
}
