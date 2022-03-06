FFX (Filters Unlimited) file format
===================================
    
    FFXFile = {
        String         fileVersion        // "FFX1.0" or "FFX1.1" or "FFX1.2"
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
        char[]         szFilesize         // human readable file size (numeric string) with NUL terminator
        char[]         szUnknown          // TODO: ??? Always 1 NUL character?
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
        int32_t        unknown1           // TODO: ??? Is usually 0x0000006e (110) for FFX 1.1 and 0x0000006b (107) for FFX 1.2
        int32_t        unknown2           // TODO: ??? Is usually 0x00000053 (94)
        int32_t        jpegThumbnailLength
        byte[]         jpegThumbnailData
    }
