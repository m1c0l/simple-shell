#!/usr/bin/env bash

echo "===TEST 1==="
echo "===bash==="
time benchmarks/1-bash
echo "===simpsh==="
benchmarks/1-simpsh
echo "===execline==="
time benchmarks/1-execline

echo "===TEST 2==="
echo "===bash==="
time benchmarks/2-bash
echo "===simpsh==="
benchmarks/2-simpsh
echo "===execline==="
time benchmarks/2-execline

echo "===TEST 3==="
echo "===bash==="
time benchmarks/3-bash
echo "===simpsh==="
benchmarks/3-simpsh
echo "===execline==="
time benchmarks/3-execline
