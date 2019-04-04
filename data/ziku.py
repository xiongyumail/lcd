#encoding: utf-8
import os
import pygame

chinese_dir = 'chinese'
if not os.path.exists(chinese_dir):
    os.mkdir(chinese_dir)

pygame.font.init()
start,end = (0x4E00, 0x9FA5) # 汉字编码范围
codepoint = 0x4E00
# for codepoint in range(int(start), int(start)+1):
word = unichr(codepoint)
font = pygame.font.Font("/usr/share/fonts/wenquanyi/wqy-microhei/wqy-microhei.ttc", 16)
# 当前目录下要有微软雅黑的字体文件msyh.ttc,或者去c:\Windows\Fonts目录下找
# 64是生成汉字的字体大小
font.size(word)
rtext = font.render(word, True, (0, 0, 0), (255, 255, 255))
#pygame.image.save(rtext, os.path.join(chinese_dir, str(codepoint) + ".bmp"))
print (pygame.image.tostring(rtext, "RGB", False))