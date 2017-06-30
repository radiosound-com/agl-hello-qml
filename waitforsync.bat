@echo off
:waitforit
if not exist "build-agl\package\hello_qml.wgt" goto waitforit
