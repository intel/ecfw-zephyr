PI_GEN Version 6.0 2019-10-21

SPI Image Utility - 
    > everglades_spi_gen_RomE.exe 
            -i <cfg_file_name> 
                -o <output_spi_file_name> 
                    -m <merge_file>

Running "everglades_spi_gen_RomE.exe" from command prompt will take 
    "spi_cfg.txt" as a default configuration file and 
    generators the output "spi_image.bin"

Other options:
==============    
    -i cfg_file_name 
        Specifies the text config file for the SPI chip & images.
        Defaults to spi_cfg.txt

    -o output_spi_file_name
        Specifies the SPI binary output file name.
        Defaults to spi_image.bin

    -m merge_file
        Read merge file as an existing SPI binary image and create FW images 
        inside it.
        No default value

        
Configuration Details:
======================        
Everglades SPI Image Generator configuration file 

SPI Configuration:
-----------------
    [SPI]
    ;SPI Flash Image SIze in Megabits 128 =>  16MB; 256 = 32 MB
    SPISizeMegabits = 128
    
    ; Use this flag if Flashmap is present(Only for SHD SPI) ignore from the 
    ; config file if not  applicable, default is False turned off 
    Flashmap = false / true
    
    ;If Flashmap = true then include the Flash Map address for SHD SPI interface
    ; Base address for the Second Chip Select if supported. 
    FlshmapAddr =<hex value DWord>
    
Device Configuration:
--------------------
    [DEVICE]
    ;Tag0 address for the image header to be located   
    TagAddr0 = <hex value Byte>
    
    ;Tag1 address for the image header to be located   
    TagAddr1 = <hex value Byte> 
    
    ; Device Paltform ID specifies the paltform intended for the code bring-up
    BoardID = <hex value Word>
    
Image Details at Tag 0
    [IMAGE "0"]
    ;Image Vendor Identification string
    HeaderVendorID = MCHP
    
    ;Image Header Version
    HeaderVersion = 02
    
    ;Image location in the SPI Flash
    ImageLocation = <Hex value Dword>
    
    ;SPI read frequency supported 12 16 24 48 in Mhz
    SpiFreqMHz = select any frequency from above 
    
    ; SPI Read mode configuration supported "slow" or "fast" or "dual" or "quad"
    SpiReadCommand = slow / fast / dual / quad
    
    ; SPI pin drive strength: 2, 4, 8, or 12 mA
    SpiDriveStrength = Select Drive strength from above list
    
    ; SPI pin slew rate slow(false) or fast (true)
    SpiSlewFast = false / true
    
    ; SPI Signal Control values default is 00
    ; bit[0] SPI Clock Polarity, corresponds to QMSPI Mode register bit [8] 
    ;        1=SPI Clock starts High
    ;        0=SPI Clock starts Low
    ; bit[1] if SPI Clock Polarity Bit[0] above is 1 
    ;           1=Data changes on the falling edge of the SPI clock
    ;           0=Data changes on the rising edge of the SPI clock
    ;        if SPI Clock Polarity Bit[0] above is 0 
    ;           1=Data changes on the rising edge of the SPI clock
    ;           0=Data changes on the falling edge of the SPI clock
    ; bit[2] if SPI Clock Polarity Bit[0] above is 1 
    ;           1=Data are captured on the rising edge of the SPI clock
    ;           0=Data are captured on the falling edge of the SPI clock
    ;        if SPI Clock Polarity Bit[0] above is 0
    ;           1=Data are captured on the falling edge of the SPI clock
    ;           0=Data are captured on the rising edge of the SPI clock
    SpiSignalControl = 0x00
    
    ; These header VTRx Level Bit values will take precedence and overwrite 
    ; any previous programmed VTRx_Levels.  If not enabled, no change to the 
    ; current VTRx bit value. By default VTRx values will be 3.3V  or false
    VTR1pinSrc18 = false / true
    VTR2pinSrc18 = false / true
    VTR3pinSrc18 = false / true

    ; Enable Authentication of Header and Firmware FW 
    ; Generate ECDSA signature of 64-byte Header, padded FW binary + optional 
    ; encryption key header. If false bytes[31:0] of the signatures contain the 
    ; SHA256(object)
    UseECDSA = false / true
    
    ; Authentication Key selection - use the valid key index to be used from the 
    ; key bank in the SPI. Valid Key select offset values  = 0 - 31
    ; Value 0 - indicates Key select @ 0 offset 
    AuthenticateKeySelt = <Key select offset>
    
    ; Auto Key revocation Enable to revoke the key bits in the OTP by Bootrom.
    ; By default Auto Key revovation is disabled or false
    AutoKeyRevEn = false / true
    
    ; Key revocation persmission. Each bit will represent to the corresponding 
    ; Key offset and the permission to revoke them. Total supported keys = 32
    KeyRevPermission = <Hex value DWord>
    
    ; Auto Roll Back protection enable bit. set to true \ false for the feature
    ; Enable. Default is disable or false
    AutoRollBackProtEn = false / true
    
    ; Roll Back Protection Permission to enable the feature. Total 128 images 
    ; will be supported and each bit represent the particular version of image
    ; RollbackProtPerm031000 - is for images 31 - 0
    ; RollbackProtPerm063032 - is for images 32 - 63
    ; RollbackProtPerm095063 - is for images 64 - 95
    ; RollbackProtPerm127096 - is for images 96 - 127
    ; All values are in hex DWord
    RollbackProtPerm031000 = <Hex value DWord>
    RollbackProtPerm063032 = <Hex value DWord>
    RollbackProtPerm095063 = <Hex value DWord>
    RollbackProtPerm127096 = <Hex value DWord>
    
    ; Tagx Build number of the Application
    TagBuildNumber = <Hex value Word>
    
    ; Current Version of the image - Valid values are from  0 - 127 bit[6:0]
    ImageRevision = <Currrent revision number>
    
    ; This EC key pair is used to sign and verify/authenticate the FW Image Header, 
    ; FW + optional key header optional key header.
    ; EC Private Key in PEM encoded Openssl SSLeay encrypted format
    ; This key is used to sign the Header and is NOT stored in the MEC chip.
    ECDSAPrivKeyFile = Authentication Key.pem 
    ECDSAPrivKeyPassword = PASSWORD for the Private Key

    ; Header Flag for verifying Authentication enable and Disable
    ; if proper keys are programmed in efuse - this flag can enable or disable
    ; authentication. If Authentication bit is set in Efuse this is don't care
    FwAuthtic = false / true
    
    ;To Encrypt Application binary using AES-256-CBC
    FwEncrypt = false / true
    
    ; FW may be AES-256-CBC encrypted
    ; The key is auto-generated and exchanged with the ROM using a procedure 
    ; based on ECDH.
    ; An EC Public Key is used by this program to Generate the AES-256 Key/IV 
    ; and a 64-byte key header appended to the encrypted FW binary. 
    ; The corresponding EC Private key is stored in the MEC chip and is used by 
    ; ROM to re-generated the AES-256 Key & IV.
    AesGenECPubKeyFile = <Encryption Certificate file >
    
    ;Firmware Application binary image
    FwBinFile = <Application Binary file . bin>
    
    ; Offset of 0 means append to end of header. Non-zero value locates FW at ImageLocation + FWOffset
    FwOffset = 0 
    
    ;Application firmware load address in SRAM
    FwLoadAddress = <Load address in Hex>
    
    ;Zero means get entry address from offset 0x4 of input Application binary which is the reset handler address
    ;If FWEntryAddress is non-zero then use it as entry point.
    FwEntryAddress = 0 or <Entry address in Hex if known>
    ;MCHP Dual signature enable
    UseMCHPECDSA = false / true
    
    ;If Dual signature is enabled provide the key and password for the keys
    MCHPECDSAPrivKeyFile = Signature Private Key.pem 
    MCHPECDSAPrivKeyPassword = PASSWORD for the Private Key
        
¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤
                                E.N.D  O.F  D.O.C.U.M.E.N.T
¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤
