# OBS Time Source

An OBS Studio plugin that allows you to add current date and time to scenes.

![screenshot](https://files.krystianch.com/obs-time-source.png)

[![builds.sr.ht status](https://builds.sr.ht/~krystianch/obs-time-source.svg)](https://builds.sr.ht/~krystianch/obs-time-source?)

## Dependencies

* Meson (`meson`)
* A ninja-compatible build tool (e.g. `samurai`)
* A C compiler  (e.g. `gcc`)
* C standard library headers (e.g. `musl-dev`)
* OBS Studio (`obs-studio-dev`)
* PangoCairo (`pango-dev`)

## Building

```sh
meson setup build
ninja -C build
```

## Installing

```sh
mkdir -p ~/.config/obs-studio/plugins/time-source/bin/64bit/
cp build/time-source.so ~/.config/obs-studio/plugins/time-source/bin/64bit/

mkdir -p ~/.config/obs-studio/plugins/time-source/data/locale/
cp data/locale/* ~/.config/obs-studio/plugins/time-source/data/locale/
```

## Uninstalling

```sh
rm -rf ~/.config/obs-studio/plugins/time-source/
```

## Contributing

Please send patches and bug reports to <~krystianch/public-inbox@lists.sr.ht>.
