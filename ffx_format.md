FFX (Filters Unlimited) file format
===================================
    
    FFXFile_V10 = {
        String         fileVersion        // "FFX1.0"
        String         title
        String         category
        String         author
        String         copyright
        String[5]      channels           // I (Intro), R, G, B, A
        SliderInfo[8]  sliders
        int32_t        gradientIndex
    }
    
    FFXFile_V11 = {
        String         fileVersion        // "FFX1.1"
        String         title
        String         category
        String         author
        String         copyright
        String[5]      channels           // I (Intro), R, G, B, A
        SliderInfo[8]  sliders
        int32_t        gradientIndex
        int32_t        numPresets         // new in FFX 1.1
        Preset[]       presets            // new in FFX 1.1
    }
    
    FFXFile_V12 = {
        String         fileVersion        // "FFX1.2"
        String         title
        String         category
        String         author
        String         copyright
        String[5]      channels           // I (Intro), R, G, B, A
        SliderInfo[8]  sliders            // Slider names prefix: {C} = Checkbox, {S} or none = Slider
        int32_t        gradientIndex
        int32_t        bitmapInfoSize
        byte*          bitmapInfo         // Data: human readable file size (numeric string), 2x NUL, JPG data
        int32_t        numPresets         // new in FFX 1.1
        Preset[]       presets            // new in FFX 1.1
    }
    
    String = {
        int32_t        length
        char[]         value
    }
    
    SliderInfo = {
        int32_t        nameLength
        char[]         name
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
