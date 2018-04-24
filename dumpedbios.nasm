00000000  9C                pushfw
00000001  80FC0F            cmp ah,0xf
00000004  7506              jnz 0xc
00000006  E8E760            call word 0x60f0
00000009  E9BC00            jmp word 0xc8
0000000C  80FC1A            cmp ah,0x1a
0000000F  7506              jnz 0x17
00000011  E8736E            call word 0x6e87
00000014  E9B100            jmp word 0xc8
00000017  80FC0B            cmp ah,0xb
0000001A  7506              jnz 0x22
0000001C  E80158            call word 0x5820
0000001F  E9A600            jmp word 0xc8
00000022  3D0311            cmp ax,0x1103
00000025  7506              jnz 0x2d
00000027  E8C167            call word 0x67eb
0000002A  E99B00            jmp word 0xc8
0000002D  80FC12            cmp ah,0x12
00000030  753E              jnz 0x70
00000032  80FB10            cmp bl,0x10
00000035  7506              jnz 0x3d
00000037  E8756C            call word 0x6caf
0000003A  E98B00            jmp word 0xc8
0000003D  80FB30            cmp bl,0x30
00000040  7506              jnz 0x48
00000042  E88E6C            call word 0x6cd3
00000045  E98000            jmp word 0xc8
00000048  80FB31            cmp bl,0x31
0000004B  7505              jnz 0x52
0000004D  E8D66C            call word 0x6d26
00000050  EB76              jmp short 0xc8
00000052  80FB32            cmp bl,0x32
00000055  7505              jnz 0x5c
00000057  E8EE6C            call word 0x6d48
0000005A  EB6C              jmp short 0xc8
0000005C  80FB33            cmp bl,0x33
0000005F  7505              jnz 0x66
00000061  E8026D            call word 0x6d66
00000064  EB62              jmp short 0xc8
00000066  80FB34            cmp bl,0x34
00000069  754F              jnz 0xba
0000006B  E81C6D            call word 0x6d8a
0000006E  EB58              jmp short 0xc8
00000070  3D1B10            cmp ax,0x101b
00000073  7445              jz 0xba
00000075  80FC10            cmp ah,0x10
00000078  7505              jnz 0x7f
0000007A  E89A60            call word 0x6117
0000007D  EB49              jmp short 0xc8
0000007F  80FC4F            cmp ah,0x4f
00000082  7536              jnz 0xba
00000084  3C03              cmp al,0x3
00000086  7505              jnz 0x8d
00000088  E8589A            call word 0x9ae3
0000008B  EB3B              jmp short 0xc8
0000008D  3C05              cmp al,0x5
0000008F  7505              jnz 0x96
00000091  E8239D            call word 0x9db7
00000094  EB32              jmp short 0xc8
00000096  3C06              cmp al,0x6
00000098  7505              jnz 0x9f
0000009A  E8479D            call word 0x9de4
0000009D  EB29              jmp short 0xc8
0000009F  3C07              cmp al,0x7
000000A1  7505              jnz 0xa8
000000A3  E88B9D            call word 0x9e31
000000A6  EB20              jmp short 0xc8
000000A8  3C08              cmp al,0x8
000000AA  7505              jnz 0xb1
000000AC  E8B09D            call word 0x9e5f
000000AF  EB17              jmp short 0xc8
000000B1  3C0A              cmp al,0xa
000000B3  7505              jnz 0xba
000000B5  E8DF9D            call word 0x9e97
000000B8  EB0E              jmp short 0xc8
000000BA  06                push es
000000BB  1E                push ds
000000BC  60                pushaw
000000BD  BB00C0            mov bx,0xc000
000000C0  8EDB              mov ds,bx
000000C2  E86434            call word 0x3529
000000C5  61                popaw
000000C6  1F                pop ds
000000C7  07                pop es
000000C8  9D                popfw
000000C9  CF                iretw
000000CA  0000              add [bx+si],al
000000CC  0004              add [si],al
000000CE  00B8FF02          add [bx+si+0x2ff],bh
000000D2  0100              add [bx+si],ax
000000D4  0004              add [si],al
000000D6  00B8FF02          add [bx+si+0x2ff],bh
000000DA  0200              add al,[bx+si]
000000DC  0004              add [si],al
000000DE  00B8FF02          add [bx+si+0x2ff],bh
000000E2  0300              add ax,[bx+si]
000000E4  0004              add [si],al
000000E6  00B8FF02          add [bx+si+0x2ff],bh
000000EA  0401              add al,0x1
000000EC  0202              add al,[bp+si]
000000EE  00B8FF01          add [bx+si+0x1ff],bh
000000F2  050102            add ax,0x201
000000F5  0200              add al,[bx+si]
000000F7  B8FF01            mov ax,0x1ff
000000FA  06                push es
000000FB  0102              add [bp+si],ax
000000FD  0100              add [bx+si],ax
000000FF  B8FF01            mov ax,0x1ff
00000102  07                pop es
00000103  0001              add [bx+di],al
00000105  0400              add al,0x0
00000107  B0FF              mov al,0xff
00000109  000D              add [di],cl
0000010B  0104              add [si],ax
0000010D  0400              add al,0x0
0000010F  A0FF01            mov al,[0x1ff]
00000112  0E                push cs
00000113  0104              add [si],ax
00000115  0400              add al,0x0
00000117  A0FF01            mov al,[0x1ff]
0000011A  0F0103            sgdt [bp+di]
0000011D  0100              add [bx+si],ax
0000011F  A0FF00            mov al,[0xff]
00000122  1001              adc [bx+di],al
00000124  0404              add al,0x4
00000126  00A0FF02          add [bx+si+0x2ff],ah
0000012A  1101              adc [bx+di],ax
0000012C  0301              add ax,[bx+di]
0000012E  00A0FF02          add [bx+si+0x2ff],ah
00000132  1201              adc al,[bx+di]
00000134  0404              add al,0x4
00000136  00A0FF02          add [bx+si+0x2ff],ah
0000013A  1301              adc ax,[bx+di]
0000013C  050800            add ax,0x8
0000013F  A0FF03            mov al,[0x3ff]
00000142  6A01              push byte +0x1
00000144  0404              add al,0x4
00000146  00A0FF02          add [bx+si+0x2ff],ah
0000014A  17                pop ss
0000014B  17                pop ss
0000014C  1818              sbb [bx+si],bl
0000014E  0405              add al,0x5
00000150  06                push es
00000151  07                pop es
00000152  0D0E11            or ax,0x110e
00000155  121A              adc bl,[bp+si]
00000157  1B1C              sbb bx,[si]
00000159  1D3F3F            sbb ax,0x3f3f
0000015C  3F                aas
0000015D  FF00              inc word [bx+si]
0000015F  0000              add [bx+si],al
00000161  0000              add [bx+si],al
00000163  0000              add [bx+si],al
00000165  0000              add [bx+si],al
00000167  0000              add [bx+si],al
00000169  0000              add [bx+si],al
0000016B  0000              add [bx+si],al
0000016D  0000              add [bx+si],al
0000016F  0000              add [bx+si],al
00000171  0000              add [bx+si],al
00000173  0000              add [bx+si],al
00000175  0000              add [bx+si],al
00000177  0000              add [bx+si],al
00000179  0000              add [bx+si],al
0000017B  0000              add [bx+si],al
0000017D  0000              add [bx+si],al
0000017F  0000              add [bx+si],al
00000181  0000              add [bx+si],al
00000183  0000              add [bx+si],al
00000185  0000              add [bx+si],al
00000187  0000              add [bx+si],al
00000189  0000              add [bx+si],al
0000018B  0000              add [bx+si],al
0000018D  0000              add [bx+si],al
0000018F  0000              add [bx+si],al
00000191  0000              add [bx+si],al
00000193  0000              add [bx+si],al
00000195  0000              add [bx+si],al
00000197  0000              add [bx+si],al
00000199  0000              add [bx+si],al
0000019B  0000              add [bx+si],al
0000019D  0000              add [bx+si],al
0000019F  0000              add [bx+si],al
000001A1  0000              add [bx+si],al
000001A3  0000              add [bx+si],al
000001A5  0000              add [bx+si],al
000001A7  0000              add [bx+si],al
000001A9  0000              add [bx+si],al
000001AB  0000              add [bx+si],al
000001AD  0000              add [bx+si],al
000001AF  0000              add [bx+si],al
000001B1  0000              add [bx+si],al
000001B3  0000              add [bx+si],al
000001B5  0000              add [bx+si],al
000001B7  0000              add [bx+si],al
000001B9  0000              add [bx+si],al
000001BB  0000              add [bx+si],al
000001BD  0000              add [bx+si],al
000001BF  0000              add [bx+si],al
000001C1  0000              add [bx+si],al
000001C3  0000              add [bx+si],al
000001C5  0000              add [bx+si],al
000001C7  0000              add [bx+si],al
000001C9  0000              add [bx+si],al
000001CB  0000              add [bx+si],al
000001CD  0000              add [bx+si],al
000001CF  0000              add [bx+si],al
000001D1  0000              add [bx+si],al
000001D3  0000              add [bx+si],al
000001D5  0000              add [bx+si],al
000001D7  0000              add [bx+si],al
000001D9  0000              add [bx+si],al
000001DB  0000              add [bx+si],al
000001DD  0000              add [bx+si],al
000001DF  0000              add [bx+si],al
000001E1  0000              add [bx+si],al
000001E3  0000              add [bx+si],al
000001E5  0000              add [bx+si],al
000001E7  0000              add [bx+si],al
000001E9  0000              add [bx+si],al
000001EB  0000              add [bx+si],al
000001ED  0000              add [bx+si],al
000001EF  0000              add [bx+si],al
000001F1  0000              add [bx+si],al
000001F3  0000              add [bx+si],al
000001F5  0000              add [bx+si],al
000001F7  0000              add [bx+si],al
000001F9  0000              add [bx+si],al
000001FB  0000              add [bx+si],al
000001FD  0000              add [bx+si],al
000001FF  00                db 0x00
