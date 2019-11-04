package main

import (
	"encoding/hex"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
)

func main() {
	if len(os.Args) < 3 {
		fmt.Println(fmt.Errorf("Not enough file arguments provided. Expect input and output file: go run generateFirmwareHeader.go PN5180Firmware_4.0.sfwu PN5180Firmware_4_0.h"))
		return
	}

	file := os.Args[1]
	if file == "" {
		fmt.Println(fmt.Errorf("Empty input file argument provided"))
		return
	}

	fileOut := os.Args[2]
	if fileOut == "" {
		fmt.Println(fmt.Errorf("Empty output file argument provided"))
		return
	}

	fw, err := ioutil.ReadFile(file)
	if err != nil {
		fmt.Println(err)
		return
	}

	// check magic number
	if fw[0] != 0 || fw[2] != 0XC0 {
		fmt.Println(fmt.Errorf("Magic number mismatch"))
		return
	}

	// extract version
	major := fw[5]
	minor := fw[4]

	fmt.Printf("Firmware version: %v.%v\n", major, minor)

	//
	var sb strings.Builder

	sb.WriteString(fmt.Sprintf("static const uint8_t gphDnldNfc_DlSequence%v_%v[] = {\n", major, minor))

	for i := range fw {
		if i > 0 {
			sb.WriteString(", ")

			if i%8 == 0 {
				sb.WriteString("\n")
			}
		}

		sb.WriteString("0x")
		sb.WriteString(hex.EncodeToString(fw[i : i+1]))
	}

	sb.WriteString("};\n")
	sb.WriteString(fmt.Sprintf("uint16_t gphDnldNfc_DlSeqSizeOf%v_%v = %v;\n", major, minor, len(fw)))

	ioutil.WriteFile(fileOut, ([]byte)(sb.String()), 0644)
}
