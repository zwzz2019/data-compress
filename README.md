# data-compress
lzw_huffman.c和delzw_unhuffman.c是整体以数组为传递接口，对数组进行lzw后进行huffman的压缩与解压缩。

f_lzw_huffman.c和f_delzw_unhuffman.c是文件接口，对文件进行lzw后进行huffman的压缩与解压缩。

dct.c是对输入数据，进行fdct变换后进行量化，以及反量化和idct的过程，量化根据H.264量化系数标准。

compress.c是究极版的lzw加huffman混合变换，速度快，压缩率高，文件接口。

read_mrf.c是对文件夹下的多个马蜂文件进行扫描整合成一个文件的程序，整合后易于压缩，可以行存储可以列存储，可以存成bin或者csv。

scandir.c是对scandir函数的示例，select.c是对select函数的示例。
