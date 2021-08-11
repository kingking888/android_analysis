package com.plug.xposed.base;

import java.util.HashMap;
import java.util.Map;

public class xposed_plug_common {
    public static class plug_name {
        static public String plug_name_fzzx = "plug_name_fzzx";
        static public String plug_name_360 = "plug_name_360";
        static public String plug_name_avl = "plug_name_avl";
        static public String plug_name_sub_plug_loader = "sub_plug_loader";
        static public String plug_name_pass_root = "pass_root";
    }

    public static class plug_cmd {
        static public String run_scan = "run_scan";
    }

    public static Map<Integer, String> plug_qihoo_level = new HashMap<Integer, String>() {{
        put(0, "安全");
        put(100, "谨慎");
        put(500, "篡改");
        put(400, "感染");
        put(700, "高危");
        put(101, "广告");
        put(800, "木马");
        put(200, "启动");
        put(1, "未知");
        put(300, "useby木马");
        put(600, "危险");
    }};


}
