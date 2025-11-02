/**
 * Authors: 
 * Jan Kalina
 * Daniel Dolejška
 * 
 * Usage: Debugging macro
 */

// This code is taken from IPK Project 2 assignment
// I added if statement to this code so it works for my verbose

// [IPK Project 2: Client for a chat server using the IPK25-CHAT protocol], Macro, Available: https://git.fit.vutbr.cz/NESFIT/IPK-Projects/src/branch/master/Project_2#example-of-client-logging-in-c

// Specific commit that shows the name of the author
// https://git.fit.vutbr.cz/NESFIT/IPK-Projects/commit/9ce06c6e81c7cb38176c14ef90ead5b22ee4821e

#pragma once
#include <iostream>

extern bool gverbose;

#define printf_debug(format, ...) \
    do { \
      if (gverbose) { \
        if (*#format) { \
            fprintf(stderr, "%s:%-4d | %15s | " format "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
        } \
        else { \
            fprintf(stderr, "%s:%-4d | %15s | \n", __FILE__, __LINE__, __func__); \
        } \
      } \
    } while (0)
