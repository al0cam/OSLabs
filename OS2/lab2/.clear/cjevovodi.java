package com.dot;

import java.io.*;
import java.util.Random;

public class Main {

    public static void main(String[] args) {
        int count = Integer.parseInt(args[0]);

        for(int i = 0; i<count; i++){
            String problem = null;
            while(problem == null) problem = generateProblem();
            System.out.println(problem+"=");
            BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));

            try {
                String userInput = reader.readLine();
                Float result = execBash(problem);
                if(Float.parseFloat(userInput) == result){
                    System.out.println("ISPRAVNO");
                }
                else System.out.println("NEISPRAVNO");

                if(result == null) System.out.println("SYNTAX ERROR?");

            } catch (IOException e) {
                e.printStackTrace();
            }
        }

    }

    private static Float execBash(String input) throws IOException {
        ProcessBuilder builder = new ProcessBuilder("/usr/bin/bc");
        builder.redirectErrorStream(true);
        Process process = builder.start();

        OutputStream stdin = process.getOutputStream ();
        InputStream stdout = process.getInputStream ();

        BufferedReader reader = new BufferedReader (new InputStreamReader(stdout));
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(stdin));

        input += "\n";
        writer.write(input);
        writer.flush();
        writer.close();

        String line;
        while ((line = reader.readLine ()) != null) {
            //System.out.println ("Stdout: " + line);
            return Float.parseFloat(line);
        }

        reader.close();
        return null;
    }

    private static String generateProblem(){
        String[] ops={"+", "-", "*", "/"};
        Random rand = new Random(); //instance of random class
        int upperbound = 11;
        int int1 = rand.nextInt(upperbound);
        int int2 = rand.nextInt(upperbound);
        int operator = rand.nextInt(ops.length);

        if(operator!=3 && int2 != 0)
            return int1 + ops[operator] + int2;

        return null;
    }
}
