#Script:        caesar_cipher.py
#Desc:          This script will encrypt, decrypt and print list of possible messages from input.
#Auth:          Pawel Ruder
#Created:       08/10/2017

import string
import time

def main():
    print("\n Caesar cipher script, please enter your choice: ") 
    ans=True
    while ans:
        print ("""
        1. Encrypt.
        2. Decrypt.
        3. Brute force - print table of possible solutions.
        4. Exit/Quit.
        """)
        ans=input('Choice:\n')
        if ans=="1":
            text = input('Enter text to encrypt\n').upper().replace(' ', '')
            shift = int(input('Enter shift\n'))
            print('Encrypted message:\n', encrypt(text, shift))
        elif ans=="2":
            text = input('Enter text to decrypt\n').upper().replace(' ', '')
            shift = int(input('Enter shift\n'))
            print('Decrypted message:\n', decrypt(text, shift))
        elif ans=="3":
            text = input('Enter encrypted message to create a table:\n').upper().replace(' ', '')
            start = time.time()
            for i in range(len(string.ascii_uppercase)):
                print(i, brute_force(text, i))
            end = time.time()
            print('\nTime:\n', end - start)        
        elif ans=="4":
            print("Goodbye!")
            break
        elif ans !="":
            print("\nNot valid choice, try again")
    
def encrypt(text, shift):
    """Encrypt input text.

    Keyword arguments:
    text -- define text, then convert it to uppercase characters
    shift -- define value of shift of the list
    upper_string -- create string that contains uppercase ascii characters
    rotate -- create list from 'upper string', slice it by value in 'shift',
        take n characters from begining and append to the end
    maketrans(x, y=None, z=None, /)
        Return a translation table usable for str.translate().
    """

    upper_string = string.ascii_uppercase
    upper_list = list(upper_string)
    rotate = upper_list[shift:] + upper_list[:shift]
    upper_rot = ''.join(rotate)
    return text.translate(str.maketrans(upper_string, upper_rot))
    
def decrypt(text, shift):
    """Decrypt input text.

    Keyword arguments:
    text -- define text, then convert it to uppercase characters
    shift -- define value of shift of the list
    upper_string -- create string that contains uppercase ascii characters
    rotate -- create list from 'upper string', slice it by value in 'shift',
        take n characters from begining and append to the end
    maketrans(x, y=None, z=None, /)
        Return a translation table usable for str.translate().
    """
    upper_string = string.ascii_uppercase
    upper_list = list(upper_string)
    rotate = upper_list[-shift:] + upper_list[:-shift]
    upper_rot = ''.join(rotate)
    return text.translate(str.maketrans(upper_string, upper_rot))

def brute_force(text, shift):
    """Decrypt input text.

    Keyword arguments:
    text -- define text, then convert it to uppercase characters
    shift -- define value of shift of the list
    upper_string -- string.ascii_uppercase -- a string containing all ASCII uppercase letters
    rotate -- create list from 'upper string', slice it by value in 'shift',
        take n characters from begining and append to the end
    maketrans(x, y=None, z=None, /)
        Return a translation table usable for str.translate().
    """
    upper_string = string.ascii_uppercase               
    upper_list = list(upper_string)                     
    rotate = upper_list[shift:] + upper_list[:shift]  
    upper_rotate = ''.join(rotate)
    return text.translate(str.maketrans(upper_string, upper_rotate))
    
if __name__ == '__main__':
    main()
