#!/usr/bin/python

import pygame
import pygame.camera
from pygame.locals import *
import os
from PIL import Image
#import antigravity
import time

resolution = (800,600)

pygame.init()
pygame.camera.init()

lcd = pygame.display.set_mode(resolution)

cam = pygame.camera.Camera("/dev/video1",resolution,"RGB")
cam.start()

thresholded = pygame.surface.Surface(resolution)
vpath = pygame.surface.Surface(resolution)
vpath.set_colorkey((0,0,0))

crect = pygame.Rect(0,0,20,20)
ccolor = (0,0,0)
lastPos = (0,0)

# get color
going = True
while going:
    events = pygame.event.get()
    for e in events:
        if (e.type == MOUSEBUTTONDOWN):
            pos = pygame.mouse.get_pos()
            crect.center = pos
            lastPos = pos
            ccolor = pygame.transform.average_color(image, crect)
            vpath.fill((0,0,0)) 
            #pygame.draw.rect(vpath,(255,0,0),crect)

        if e.type == QUIT or (e.type == KEYDOWN and e.key == K_ESCAPE):
            going = False

        if (e.type == KEYDOWN and e.key == K_w):
            timestr = time.strftime("%Y%m%d-%H%M%S")
            pygame.image.save(lcd,"motion_" + timestr + ".jpg")
            pygame.image.save(vpath,"mask_" + timestr + ".jpg")


    if cam.query_image():
        image = cam.get_image()

        # (80,80,80) was too much
        mask = pygame.mask.from_threshold(image, ccolor, (50,50,50))
        lcd.blit(image, (0,0))
        connected = mask.connected_component()
        if connected.count() > 10:
            coord = connected.centroid()
            #pygame.draw.line(vpath, (0,255,0), lastPos, coord, 5)
            pygame.draw.line(vpath, (0,255,0), lastPos, coord, 3)
            lastPos = coord

        lcd.blit(vpath, (0,0))
        pygame.display.flip()
