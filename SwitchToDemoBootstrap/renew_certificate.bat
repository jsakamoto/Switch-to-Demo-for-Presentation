@echo off
pushd "%~dp0"
openssl req -new -x509 -key SwitchToDemoBootstrap_TemporaryKey.key -out SwitchToDemoBootstrap_TemporaryKey.crt -config .\openssl.cnf -days 365
openssl pkcs12 -export -nodes -inkey SwitchToDemoBootstrap_TemporaryKey.key -in SwitchToDemoBootstrap_TemporaryKey.crt -out SwitchToDemoBootstrap_TemporaryKey.pfx