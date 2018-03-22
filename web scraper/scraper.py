#Script:        scraper.py
#Desc:          Website parse script
#Auth:          Pawel Ruder
#Created:       21/11/2017
#References:    password database: https://hashes.org/leaks.php?id=106


import sys, re, os
import urllib.request
import hashlib
import collections
import time
import funlib

def file_len_counter(fname):
  
    '''This function takes file that contains hash-plain database and count its lines number.'''
    
    f = open(fname)
    lines = sum(1 for line in f)
    f.close()
    return lines

def badhash(md5val, file):

    '''This function proccess two tasks: finds badfiles extracted from website, and finds MD5 signatures in hash-plain file'''

    with open(file, 'r') as f:
        for line in f:
            reg = re.findall("^'?([0-9A-Fa-f]{32})'?:'?([^\s]+)'?", line)
            if reg and md5val in reg[0]:
                return (reg[0][1])

def hasher():

    '''This function analise downloaded files and create a list of MD5 hashes from file content'''

    listing = []
    source = sys.argv[2]
    for i in os.listdir(source):
        hsh = hashlib.md5()
        with open(source + i, 'rb') as hashfile:
            buf = hashfile.read()
            hsh.update(buf)
        listing.append(hsh.hexdigest())
    return listing

def looper(func, msg, msg2):

    '''This function takes three arguments: "function name", "message" and "message2". Rather than repeat the same proccess for every function separately in main, this one do this automatically.'''

    if func:
        qty = len(func)
        logger(msg.format(qty))
        for i in func:
            logger(i)
            print(msg2.format(i))
    else:
        logger(msg + 'No results.')
        print(msg + 'No results.\n')

def file_ext():

    '''This function parse the directory where downloaded files are stored, read 3 bytes from begining of the file and compare created list with list_sigs dictionary.'''
    
    print('\n[*] File types to extension match:\n')
    file_sigs = {b'\xff\xd8\xff':('.jpg','.jpeg','.JPG','.JPEG'),
                 b'\x47\x49\x46':('.gif','.GIF'),
                 b'\x89\x50\x4e':('.png','.PNG'),
                 b'\x50\x4b\x03':('.docx','.DOCX'),
                 b'\x42\x4d\x36':('.bmp','.BMP'),
                 b'\x25\x50\x44':('.pdf','.PDF')}
    inf = []
    for i in os.listdir(sys.argv[2]):
        file = open(sys.argv[2]+i, "rb")
        sig = file.read(3)
        file.close()
        ext = os.path.splitext(i)[1]
        if sig in file_sigs.keys():
            if ext in file_sigs[sig]:
                inf.append('Matching extension: {} for file: {}.'.format(ext, i))
            else:
                inf.append('Extension not match or not found for file: {}. It is a {} file.'.format(i, file_sigs[sig][0]))
        else:
            inf.append('No such signature in database.')
                
    return inf

def logger(*args):

    '''This function takes any number of arguments, join them and appends to the output file'''
    
    with open(sys.argv[3], "a+") as file:
          file.write("".join(args) + '\n')
        
def file_download(dirc, file):

    '''This function takes two arguments: "dirc" which is a destination folder to store files, and "file" which is a file name specified in the function list.'''
      
    sitepath = sys.argv[1] + file
    filename = file.split('/')[-1]

    try:
        #download if not exists or...
        if not os.path.exists(dirc + file) or not os.path.exists(dirc + filename): 
            if not re.findall('http', file):
                urllib.request.urlretrieve(sitepath, dirc + filename)
                print ('Downloaded file {}.'.format(filename))
            elif re.findall('http', file):
                urllib.request.urlretrieve(file, dirc + filename)
                print ('Downloaded file {}.'.format(filename))
                
        #...if file exists append a incremeting number everytime file is found.
        else:
            i = 1
            alt = file.split('.')
            alt.insert(1, '{}.')
            nfile = ''.join(alt)
            while os.path.exists(dirc + nfile.format(i)):
                i += 1
            urllib.request.urlretrieve(sitepath, dirc + nfile.format(i)) or urllib.request.urlretrieve(file, dirc + nfile.format(i))
            print('File {} exist, renamed.'.format(filename.format(i)))
            
    #error handling
    except Exception as e:
        print("Couldn't download the file: {}. Error message: {}.".format(filename, e))

def main():

    # append following arguments:
    sys.argv.append('http://www.soc.napier.ac.uk/~40009856/CW/')        # arg 1
    #sys.argv.append('https://arstechnica.co.uk/')  
    sys.argv.append(r'C:/temp/cw/downloads/')                           # arg 2
    sys.argv.append(r'C:/temp/cw/output.txt')                           # arg 3
    
    badfiles = 'C:/Users/Admin/Desktop/final/badfiles.txt'
    md5lookup = 'C:/Users/Admin/Desktop/final/106_17-media_found_hash_plain.txt'
    
    if len(sys.argv) != 4:
        print ('[-] Insufficient arguments\n'+'[-] Example: [py scraper.py] [source URL] [dest for downloaded files] [dest for log file/filename]')
        return

    try:

        if not os.path.exists(sys.argv[2]):
            print('No directory found, creating directory:',sys.argv[2],'\n')
            os.makedirs(sys.argv[2])
        else:
            print('Directory',sys.argv[2],'exist\n')

        websrc = funlib.wget(sys.argv[1])

        #print(hasher())
        
        # call looper function which repeat logger function task and append results to log file.
        print('[*] Website analysis\n')
        print('[{}] URLs found:\n'.format(len(funlib.urls(websrc))))
        looper(funlib.urls(websrc), '\n[{}] URLs found:\n', 'Extracted URL: {}.')
        print('\n[{}] E-mail addresses found:\n'.format(len(funlib.e_mail(websrc))))                         
        looper(funlib.e_mail(websrc), '\n[{}] E-mail addresses found:\n', 'Extracted email: {}.')
        print('\n[{}] Phone numbers found:\n'.format(len(funlib.phone_numbers(websrc))))            
        looper(funlib.phone_numbers(websrc), '\n[{}] Phone numbers found:\n', 'Extracted phone number: {}.')
        print('\n[{}] MD5 hashes found:\n'.format(len(funlib.md5_hashes(websrc))))   
        looper(funlib.md5_hashes(websrc), '\n[{}] MD5 hashes found:\n', 'Extracted hash: {}.')  

        #call hasher function to find duplicates using list comprehension
        duplicates = [i for i, count in collections.Counter(hasher()).items() if count > 1]
        if duplicates:
            print('\n[*] Found duplicate file signature:\n {}'.format(duplicates))
        else:
            print('\n[*] No duplicate files found\n')

        # download images and append results to log file.
        print('[*] Analysis and download of website contents:\n')
        logger('\n[{}] Image files found:\n'.format(len(funlib.img_file(websrc))))
        for img in funlib.img_file(websrc):                                                
            file_download(sys.argv[2], img)
            logger(sys.argv[1], img)

        # download documents and append results to log file.
        logger('\n[{}] Document files found:\n'.format(len(funlib.doc_file(websrc))))                                            
        for doc in funlib.doc_file(websrc):
            file_download(sys.argv[2], doc)
            logger(sys.argv[1], doc)

        looper(file_ext(), '\n[*] File types to extension match:\n', '{}')              

        # call hasher function to find badfiles using badhash function, then call logger function to log results.
        print('\n[*] Bad files analysis:\n')
        logger('\n[*] Badfiles lookup:\n')
        for md5 in hasher():                                                                
            if badhash(md5, badfiles):
                logger('{} checksum match badfile: {}'.format(md5, badhash(md5, badfiles)))
                print('Hash {} found for {}'.format(md5, badhash(md5, badfiles.replace("'", ""))))

        # call funlib.md5_hashes function to find hidden passwords reusing badhash function, then call logger function to log results.
        # then call file_len_counter function to calculate speed of searching in the passwords database.
        start = time.time()
        print('\n[*] Dictionary cracking, this may take a minute:\n')
        logger('\n[*] MD5 lookup:\n')
        if funlib.md5_hashes(websrc):
            for md5p in funlib.md5_hashes(websrc):
                if badhash(md5p, md5lookup):
                    logger('{} checksum match password: {}'.format(md5p, badhash(md5p, md5lookup)))
                    print('Extracted hash {} match password: {}'.format(md5p, badhash(md5p, md5lookup)))
                else:
                    print('No matching password found')
            end = time.time()
            avg = round(file_len_counter(md5lookup) / (end - start), 2)
            msec = round(1000 / avg, 4)
            print('\nPerformance: {} H/s ({}ms).\n\nDone!'.format(avg, msec))
        else:
            print('Failed to find any hashes.\n\nDone!')

    # error handling.
    except Exception as e:
        #if WindowsError:
          #print('Directory or path error, please enter path in Windows format, e.g C:/temp/')
        #else:
        print(e)

if __name__ == '__main__':
    main()
