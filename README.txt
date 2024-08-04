This is a 960x704 example running on the Dreamcast (confirmed to be working through OSSC)

If you want to try this on your OSSC as well, use the following settings :
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

This will not work on (most) CRT monitors because the blanking intervals this uses are too short (and using larger ones would mean decreasing the resolution).
This will also not work on most LCD PC monitors but for a different reason : most of them don't support the low refresh rate (36hz).
You'll get better luck with an LCD monitor that specfically supports 24hz or a TV.

### Compiling

```
make 
make pack
```