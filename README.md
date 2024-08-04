This is a 960x704 example running on the Dreamcast (confirmed to be working through OSSC)

OSSC settings for 960x704 36hz
```
H.samplerate = 1024
H.synclen = 16
H.backporch = 32 
H.active = 960
V.synclen = 8
V.backporch = 16
V.active = 704
H.border = 0
V.border = 0
```

For 960x960 27hz
```
H.samplerate = 1032
H.synclen = 32
H.backporch = 40 
H.active = 960
V.synclen = 8
V.backporch = 31
V.active = 960
H.border = 0
V.border = 0
```

This will not work on (most) CRT monitors because the blanking intervals this uses are too short (and using larger ones would mean decreasing the resolution).
This will also not work on most LCD PC monitors but for a different reason : most of them don't support the low refresh rate (36hz).
You'll get better luck with an LCD monitor that specfically supports 24hz or a TV.

### Compiling

```
make 
make pack
```