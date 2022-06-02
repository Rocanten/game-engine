@echo off
mkdir x:\cpp\handmade\build
pushd x:\cpp\handmade\build
cl -FC -Zi x:\cpp\handmade\code\handmade.cpp User32.lib Gdi32.lib
popd
