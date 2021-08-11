package com.plug.base;

import com.units.*;

import java.io.File;
import java.io.IOException;

import com.alibaba.fastjson.*;


public class plug_config {
    public String analyse_packet_name;
    public okhttp_config okhttp;

    public plug_config() {
    }

    public static class okhttp_config {
        public boolean print_stack;
    }

    public static plug_config load_plug_config() {
        try {
            plug_config pconfig = null;
            File file = new File(config.plug_config_path);
            if (file.exists() && file.canRead()) {
                String config_data = new String(units.load_file(config.plug_config_path));
//                log.i("load config " + config_data);
                pconfig = JSONObject.parseObject(config_data, plug_config.class);
            }
            return pconfig;
        } catch (IOException e) {
//            log.i("load config_data error!!!" + e.getMessage());
            return null;
        }
    }

    public static void save_plug_config(plug_config pconfig) {
        try {
            String config_data = JSON.toJSONString(pconfig);
            units.save_file(config.plug_config_path, config_data.getBytes());
        } catch (IOException e) {
            log.i("save config_data error!!!" + e.getMessage());
        }
    }
}
