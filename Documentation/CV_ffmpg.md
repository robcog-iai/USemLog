run the following shell script in \Scans folder:

```
    $array = @("D", "L", "M", "N", "U")
    Get-ChildItem -Directory | ForEach-Object{  
        foreach ($element in $array) {         
             ffmpeg -framerate 60 -i "$($_.Name)/$($element)/img1%04d.png" "$($_.Name)_$($element).mp4" 
        }
    }
```

