package com.zpf.compress;


import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Target({ElementType.FIELD, ElementType.METHOD, ElementType.PARAMETER})
@Retention(RetentionPolicy.RUNTIME)
public @interface FileType {
    int UNKNOWN = -1;
    int OTHER = 0;
    int JPEG = 1;
    int PNG = 2;
    int WEBP = 3;
    int GIF = 4;
}
