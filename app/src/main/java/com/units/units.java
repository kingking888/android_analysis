package com.units;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class units {
    public static byte[] load_file(String fileName) throws IOException {
        String encoding = "UTF-8";
        File file = new File(fileName);
        Long len = file.length();
        byte[] data = new byte[len.intValue()];
        FileInputStream in = new FileInputStream(file);
        in.read(data);
        in.close();
        return data;
    }

    public static void save_file(String fileName, byte[] data) throws IOException {
        File file = new File(fileName);
        FileOutputStream in = new FileOutputStream(file);
        in.write(data);
        in.close();
    }

    public static List<String> get_stack() {
        StackTraceElement stack[] = Thread.currentThread().getStackTrace();
        List<String> stacks = new ArrayList<>();
        for (int i = 0; i < stack.length; i++) {
            stacks.add(stack[i].getClassName() + " 。" + stack[i].getMethodName());
        }
        return stacks;
    }

    public static void log_stack() {
        StackTraceElement[] stack = Thread.currentThread().getStackTrace();
        log.i("----------------------stack-----------------------");
        for (int i = 0; i < stack.length; i++) {
            log.i("\t\t" + i + ". " + stack[i].getClassName() + " -> " + stack[i].getMethodName());
        }
    }

}
