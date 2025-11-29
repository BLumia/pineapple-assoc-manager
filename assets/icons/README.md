To generate ico file from svg file (`app-icon.svg` to `app-icon.ico` for example):

```shell
magick -density 512x512 -background none app-icon.svg -define icon:auto-resize app-icon.ico
```

But by default, it will generate more sizes than we want. Since the generated icon (at least for app-icon.ico) will be included in the executable, we can specify the sizes we want.

```shell
magick -density 512x512 -background none app-icon.svg -define icon:auto-resize=24,64,512 app-icon.ico
```

----

Note:

1. The SVG files in this directory are directly exported by Affinity and not optimized. We don't include the SVG files in the executable resouces.
2. Only the `app-icon.ico` is included in the executable. Other icons are "installed" to `${CMAKE_INSTALL_BINDIR}/icons`, which is the default icon directory for `passoc.exe` if we leave the `icon` field empty.
