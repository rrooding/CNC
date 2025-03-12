# RootCNC Controller ISO Rev3

## Update FluidNC Firmware

Connect to the RootCNC controller using USB. 

To update the FluidNC firmware, download the latest firmware from the [FluidNC GitHub repository](https://github.com/bdring/FluidNC/releases) and upload the firmware.bin file to the RootCNC controller using the following command:

```bash
./install-wifi.sh
```

## Configure WiFi

Open FluidTerm (./fluidterm.sh) and configure WiFi:

```
$Sta/SSID=<ssid>
$Sta/Password=<password>
$Sta/IPMode=DHCP
```

The RootCNC Controller will now join the network.

## Configuration

Upload the config.yaml file to the RootCNC controller to configure the device. The file can be uploaded using the following command:

```bash
curl -F file=@config.yaml http://fluidnc.local/files
```
