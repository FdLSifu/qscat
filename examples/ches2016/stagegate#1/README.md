# How to recover the key

1. Launch qscat
2. Click top left button to open traces binary file named `2016.06.01-09.50.28_traces_x1000x9996x4.bin`
3. Click `Open DataSet` to open text binary file named `2016.06.01-09.50.28_textin_x1000x16x4.bin` 
4. Click `Attack` button
5. Here is the attack configuration
* Attack method: CPA
* Algorithm: AES
* Function: AES output SBOX
* Points Range: 0-800
* Number of traces: 1000
* Bytes Index: All (0-15 selected)
6. Click `Launch Attack`

