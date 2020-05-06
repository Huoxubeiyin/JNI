package com.showame.allies.jni;

/**
 * Created by rememberme on 2018/8/9.
 * <p>
 * Email：891904833@qq.com
 * <p>
 * Description ：
 */
public class ResultValue {

    public static int id = 100;

    private String mValue;

    ResultValue(String name) {
        mValue = name;
    }

    private boolean setV(String v) {
        mValue = v;
        return true;
    }

    public String getV() {
        System.out.println(mValue);
        return mValue;
    }

}
