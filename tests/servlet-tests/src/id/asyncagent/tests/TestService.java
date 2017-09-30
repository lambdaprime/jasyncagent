package id.asyncagent.tests;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.ForkJoinPool;

import javax.annotation.security.DenyAll;
import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.Consumes;
import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.container.AsyncResponse;
import javax.ws.rs.container.Suspended;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.Response;

import id.asyncagent.JAsyncAgent;

@Path("/")
@DenyAll
public class TestService {
    
    static {
        JAsyncAgent.start();
    }
    
    @Path("/jasyncagent")
    @GET
    @Consumes({"application/xml", "application/json"})
    public void jasyncagent(@Context HttpServletRequest request, @Suspended final AsyncResponse ar) {
        JAsyncAgent.addRunnable(() -> {
            ar.resume(send());
        });
    }
    
    @Path("/async")
    @GET
    @Consumes({"application/xml", "application/json"})
    public void async(@Context HttpServletRequest request, @Suspended final AsyncResponse ar) {
        Runnable asyncJob = () -> {
            ar.resume(send());
        };
        ForkJoinPool.commonPool().execute(asyncJob);
    }

    static Executor pool = Executors.newCachedThreadPool();
    
    @Path("/asyncpool")
    @GET
    @Consumes({"application/xml", "application/json"})
    public void asyncpool(@Context HttpServletRequest request, @Suspended final AsyncResponse ar) {
        Runnable asyncJob = () -> {
            ar.resume(send());
        };
        pool.execute(asyncJob);
    }
    
    @Path("/sync")
    @GET
    @Consumes({"application/xml", "application/json"})
    public Response sync(@Context HttpServletRequest request) {
        return Response.ok(send()).build();
    }
    
    private String send() {
        StringBuilder buf = new StringBuilder();
        try {
            URL url = new URL("http://localhost:8080/bashStub");
            //URL url = new URL("http://google.com");
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setRequestMethod("GET");
            conn.setRequestProperty("Accept", "application/json");
            conn.setDoOutput(true);
            conn.getOutputStream().write("Hello world".getBytes());
            BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
            br.lines().forEach(buf::append);
            br.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return buf.toString();
    }

}