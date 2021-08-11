package com.plug.export;

import com.alibaba.fastjson.JSON;

public class frida_helper {
    public static String object_2_string(Object obj) {
        return JSON.toJSONString(obj);
    }

    public static String byte_2_hex_str(byte[] by) {
        StringBuilder sb = new StringBuilder();
        for (byte item : by) {
            sb.append(String.format("%02x", item));
            sb.append(" ");
        }
        return sb.toString();
    }

    public static String byte_2_string(byte[] by, String decode) {
        try {
            return new String(by, decode);
        } catch (Exception e) {
            return e.toString();
        }
    }

}
