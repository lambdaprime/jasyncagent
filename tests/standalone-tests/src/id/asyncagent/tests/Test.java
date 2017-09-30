package id.asyncagent.tests;

import static java.util.stream.IntStream.range;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import id.asyncagent.JAsyncAgent;

public class Test {
    
    static class X implements Runnable {
        AtomicInteger acc = new AtomicInteger(0);
        
        @Override
        public void run() {
            System.out.println("JAVA RUNN!!!!!!!");
            try {
0                URL url = new URL("http://localhost:8080/bashStub");
                //URL url = new URL("http://google.com");
                HttpURLConnection conn = (HttpURLConnection) url.openConnection();
                conn.setRequestMethod("GET");
                conn.setRequestProperty("Accept", "application/json");
                conn.setDoOutput(true);
                conn.getOutputStream().write(String.valueOf(acc.incrementAndGet()).getBytes());
                BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
                br.lines().forEach(System.out::println);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
    
    static X x = new X();
    
    static List<String> results = new LinkedList<>();
    
    static void measure(String desc, Runnable r) {
        long l = System.currentTimeMillis();
        r.run();
        results.add(desc);
        results.add("Elapsed time: " + (System.currentTimeMillis() - l) + "ms");
        results.add("-");
    }
    
    public static void main(String[] args) throws UnknownHostException, IOException, InterruptedException {
        int n = 100;
        measure("With JAsyncAgent", () -> {
            JAsyncAgent.start();
            range(0, n).forEach(i -> JAsyncAgent.addRunnable(x));
            JAsyncAgent.stop();
        });
        System.out.println("\n\n\n---------------------------------------------");
        measure("With ForkJoinPool", () -> {
            range(0, n).forEach(i -> ForkJoinPool.commonPool().execute(x));
            try {
                ForkJoinPool.commonPool().awaitTermination(Long.MAX_VALUE, TimeUnit.NANOSECONDS);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        });
        System.out.println("\n\n\n==============================================");
        results.stream().forEach(System.out::println);
    }
    
}
