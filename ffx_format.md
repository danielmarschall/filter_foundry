FFX (Filters Unlimited) file format
===================================
    
    FFXFile = {
        String         fileVersion        // "FFX1.0", or "FFX1.1", or "FFX1.2"
        String         title
        String         category
        String         author
        String         copyright
        String[5]      channels           // I (Intro), R (Red), G (Green), B (Blue), A (Alpha)
        SliderInfo[8]  sliders
        int32_t        gradientIndex
        int32_t        bitmapInfoSize     // only in FFX >= 1.2, otherwise not present
        BitmapInfo     bitmapInfo         // only in FFX >= 1.2, otherwise not present
        int32_t        numPresets         // only in FFX >= 1.1, otherwise not present
        Preset[]       presets            // only in FFX >= 1.1, otherwise not present
    }
    
    BitmapInfo = {
        char[7]        szFilesize         // human-readable filesize in decimal notation (numeric string) with NUL terminator(s)
        char[]         jpgData
    }
    
    String = {
        int32_t        length
        char[]         value
    }
    
    SliderInfo = {
        int32_t        nameLength
        char[]         name               // In FFX >= 1.2: Slider can have names prefixes:
                                          // {C} = Checkbox
                                          // {S} or none = Slider
        byte           enabled
        int32_t        initialValue
    }
    
    Preset = {
        String         name
        int32_t[8]     sliderPosition
        int32_t        gradientIndex
        int32_t        jpegThumbnailWidth
        int32_t        jpegThumbnailHeight
        int32_t        jpegThumbnailLength
        byte[]         jpegThumbnailData
    }
