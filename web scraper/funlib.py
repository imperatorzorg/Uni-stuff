#Script:        funlib.py
#Desc:          Scraper.py function library
#Auth:          Pawel Ruder
#Created:       21/11/2017


import os, sys, re
import urllib.request

def wget(url):

    '''This function read the webpage and return its contents'''
    try:
        webpage = urllib.request.urlopen(url)
        contents = webpage.read().decode('utf-8')
        return contents
    except:
        print("Wrong URL")

def urls(source):
    
    '''This function takes extracted webpage data as argument and finds all hyperlinks'''

    src = source.replace("'" , '"')
    return set(re.findall('href=["]([\:()=/\.\w\%-\?\@]+)["]', src))

def img_file(source):

    '''This function takes extracted webpage data as argument and finds image files with specified format'''

    images = re.findall('src="([\:()=/\.\w\%-]+)"', source)
    images += re.findall('([\:()=/\.\w\%-]+\.[jpgneifJPGNEIF]{3,4})', source)
    
    return set(images)

def e_mail(source):

    '''This function takes extracted webpage data as argument and finds all e-mail addresses'''

    return set(re.findall('\w+\.?\w+?\@\w+\.?\-?\_?\w+?\.?\w+?\.\w+', source))

def phone_numbers(source):

    '''This function takes extracted webpage data as argument and finds all phone numbers'''

    phones = re.findall('[\d]{4}\s[\d]{3}\s[\d]{4}', source)
    phones += re.findall('\+\d+\-?\(?\d\)?\-?\d+\-?\s?\d+\s?\d+', source)

    return set(phones)

def md5_hashes(source):

    '''This function takes extracted webpage data as argument and finds MD5 pattern set of symbols and letters'''

    md5 = re.findall('[0-9A-Fa-f]{32}', source)
    md5.sort()
    return set([x.lower() for x in md5])

def doc_file(source):

    '''This function takes extracted webpage data as argument and finds documents with specified format'''

    docs = re.findall('([\:()=/\.\w\%-]+\.[docpxdf]{3,4})', source)

    return set(docs)
