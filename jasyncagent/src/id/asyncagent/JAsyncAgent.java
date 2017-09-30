package id.asyncagent;

public class JAsyncAgent {

    static {
        System.loadLibrary("jasyncagent");
    }
    
    public static native void start() throws JAsyncAgentException;
    public static native void stop();
    
    public static native void addRunnable(Runnable r) throws JAsyncAgentException;
        
}
