// IXposedClient.aidl
package com.caller;

// Declare any non-default types here with import statements

interface IXposedClient {
    boolean on_callback(String cmd, in byte[]  args) ;
}