# agl-hello-qml
A simple hello-world app using AGL style controls that can be run either on your desktop or on AGL

## Note for AGL quickcontrols2 style
If you'd like to run the UI on your desktop and see it in AGL style, you'll need the AGL quick controls 2 style files on your machine
`git clone https://gerrit.automotivelinux.org/gerrit/p/src/qtquickcontrols2-agl-style.git` and place the .QML and image files in, e.g. C:\Qt\5.9\mingw53_32\qml\QtQuick\Controls.2\AGL

I found this to be pretty handy for checking to see what it would look like before trying to deploy to the board, so I also added some tweaks to main.cpp and the default QML hello world app so that it'd scale on desktops whose monitors aren't that tall ;)

# Qt Creator + XDS server integration
Heard of [XDS](https://lists.linuxfoundation.org/pipermail/automotive-discussions/2017-June/004293.html) yet? If not, read there first.

Thanks to IoT.bzh for the video, which showed it off in NetBeans and inspired me to try it in Qt Creator, and thanks to Sebastien for infinite patience with me back and forth on the mailing list trying to get it up and running.

Latest user guide as of the time of this writing: http://iot.bzh/download/public/XDS/docs/XDS_UsersGuide.pdf

This is for running on Linux, as I've not been able to get XDS agent running on Windows lately. (I think support for it was removed? Not sure)

Patches welcome! ;)

## Qt Creator Options
Do these before opening the project

### Devices tab on the left
- Add: Generic Linux Device
  - Name: `My AGL board`
  - Host name: `board.ip.address.here`
  - Username: `root`
  - Password: blank (Note: this depends on the agl image being built with agl-devel. If it is not, I guess you'd need to set up ssh keys)

### Build & Run tab on the left
#### Compilers tab
- Add: Custom C++
  - Name: `AGL`
  - Compiler path: `xds-exec` (Make sure wherever you placed the executable is in your path)
  - All other fields blank/unknown
#### Kits tab
- Add
  - Name: `AGL armv7vehf` (or your particular arch)
  - Device type: Generic Linux Device
  - Device: select your device
  - Compiler:
    - C
      - No compiler
    - C++
      - AGL
  - Environment: Change...
    - ```
      XDS_SERVER_URL=http://xds.server.url:8000
      XDS_SDK_ID=abcdef
      ```
      (substitute for whatever SDK you installed - at the command line, `xds-cli sdks ls` will show you)
      
  - Debugger: None

  - Qt version: ? select whatever's available, I guess - if you select none, the project will not be able to be configured with the AGL kit.
    - TODO: add a Qt Version package that matches what the XDS is using to build? I get some warning messages in Qt Creator about things not matching up

## Clone & build project
Clone the repository, if you haven't already. In your XDS dashboard, at http://xds.server.url:8000 as you previously specified, set up a new project in the folder where you put it. You'll get a project ID.

Open the top hello\_qml.pro

### Configure with AGL kit
Check the box for the AGL kit you previously set up

### Projects tab

#### Build & Run \ AGL kit
(TODO: come up with a custom Qt Creator plugin instead of RemoteLinux to set up reasonable build & run defaults for AGL?)

##### Build
- General: set Build directory:
  ```
  ./build-agl
  ```
  (Qt Desktop doesn't like you putting build directories under source directories, but we can do what we want here)
  
- Delete all existing build and clean steps (TODO: figure out how to create a build configuration that puts these in for you?)

- Add build steps:
  - Custom process step
    - Command: `xds-exec`
    - Arguments: `-- rm -f build/package/%{CurrentProject:Name}.wgt`

  - Custom process step
    - Command: `xds-exec`
    - Arguments: `-- mkdir -p build; cd build; qmake ../`

  - Custom process step
    - Command: `xds-exec`
    - Arguments: `-- cd build; make all`

- Add clean step: Custom process step
  - Command: `xds-exec`
  - Arguments: `-- rm -r build/* build/.qmake.stash`

- Build environment:
  - Add `XDS_PROJECT_ID=ASDF-FDSA_project_id`

##### Run
- Deployment
  - Add Deploy Step: Custom Process Step
    - Command: `./waitforsync.sh`
    - Working directory: `%{CurrentProject:Path}`
    - Move this before Upload files via SFTP (it's a batch file that waits until the .wgt file exists in build-agl\package)
    
  - Add Deploy Step: Run custom remote command (after Upload files via SFTP)
    - ```
      afm-util kill `pgrep -f afb-daemon.*%{CurrentProject:Name}`
      ```
      
  - Add Deploy Step: Run custom remote command
    - ```
      afm-util install %{CurrentProject:Name}.wgt
      ```
      
- Run configuration: Add: Custom executable (on remote generic linux host)
  - Remote executable: `echo`
  - Arguments: `"Insert command line to start and show app on screen here? In the mean time, tap that app on the home screen"`
- Remove run configuration hello_qml (on Remote Device)

After you hit the green Run button in Qt Creator, check the compile output to see that it installed the .wgt file

## Run it on AGL
`vi /var/local/lib/afm/applications/windowmanager-service-2017/0.1/etc`

Add `|Hello QML` to the end of the role string (after Mixer)

(Need this in order to return back to the app after first launch--home screen sends a "tapShortcut" event)

After first install, the window manager and home screen will need to be restarted. At a terminal on the target:

```
systemctl --user restart afm-service-windowmanager-service-2017@0.1
systemctl --user restart afm-appli-homescreen-2017@0.1
```
