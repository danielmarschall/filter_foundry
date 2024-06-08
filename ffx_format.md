FFX (Filters Unlimited) file format
===================================
    
    FFXFile = {
        FFXString         fileVersion        // "FFX1.0", or "FFX1.1", or "FFX1.2"
        FFXString         title
        FFXString         category
        FFXString         author
        FFXString         copyright
        FFXString[5]      channels           // I (Intro), R (Red), G (Green), B (Blue), A (Alpha)
        FFXSliderInfo[8]  sliders
        uint32_t          gradientIndex
        uint32_t          bitmapInfoSize     // only in FFX >= 1.2, otherwise not present
        FFXBitmapInfo     bitmapInfo         // only in FFX >= 1.2, otherwise not present
        uint32_t          numPresets         // only in FFX >= 1.1, otherwise not present
        FFXPreset[]       presets            // only in FFX >= 1.1, otherwise not present
    }
    
    FFXBitmapInfo = {
        char[7]           szFilesize         // human-readable filesize in decimal notation (numeric string) with NUL terminator(s)
        char[]            jpgData
    }
    
    FFXString = {
        uint32_t          length
        char[]            value
    }
    
    FFXSliderInfo = {
        uint32_t          nameLength
        char[]            name               // In FFX >= 1.2: Slider can have names prefixes:
                                             // {C} = Checkbox
                                             // {S} or none = Slider
        byte              enabled
        int32_t           initialValue
    }
    
    FFXPreset = {
        FFXString         name
        int32_t[8]        sliderPosition
        int32_t           gradientIndex
        int32_t           jpegThumbnailWidth
        int32_t           jpegThumbnailHeight
        int32_t           jpegThumbnailLength
        byte[]            jpegThumbnailData
    }
