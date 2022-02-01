from tkinter import *
import socket
import sys
import threading
import time

IP = sys.argv[1]
PORT = 4444
ADDR = (IP, PORT)
SIZE = 128
FORMAT = "utf-8"

#łączenie z serwerem
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(ADDR)

loginArray = ["mat","pat","ala","ewa","janek","arek"]

zalogowany = 0
exit = 0
login = ""

root = Tk()
root.title('GG')

TOPPINGS = [
    (loginArray[0], loginArray[0]),
	(loginArray[1], loginArray[1]),
	(loginArray[2], loginArray[2]),
	(loginArray[3], loginArray[3]),
	(loginArray[4], loginArray[4]),
    (loginArray[5], loginArray[5]),
]

user = StringVar()
user.set("pat")

for text, topping in TOPPINGS:
    Radiobutton(root, text=text, variable=user, value=topping).pack(anchor=W)

def odbieranie():
    while(zalogowany == 0 and exit == 0):
        time.sleep(1)
    while(zalogowany == 1 and exit == 0):
        time.sleep(0.5)
        data = ":recv"
        client.send(data.encode(FORMAT))
        BigData = client.recv(128).decode(FORMAT)
        if(len(BigData) <= 1):
            client.send("end".encode(FORMAT))
            BigData = client.recv(128).decode(FORMAT)
        else:
            Output.insert(END, BigData[1:])
            client.send("check".encode(FORMAT))
            while(True):
                BigData = client.recv(128).decode(FORMAT)
                if(BigData == "false"):
                    Output.insert(END, "")
                    break
                client.send("check".encode(FORMAT))
                BigData = client.recv(128).decode(FORMAT)
                Output.insert(END, BigData)
                client.send("check".encode(FORMAT))
        
    print("koniec odbierania")

def przycisk_logowania(data): #logowanie
    if(len(data)< 1):
        return
    global zalogowany
    if(zalogowany == 1):
        return
    client.send(data.encode(FORMAT)) #wysyła login do servera
    data1 = client.recv(SIZE).decode(FORMAT) #odbiera wynik logowania: 1 - poprawnie zalogowany, j - ktoś inny już jest zalogowany pod tym samym loginem, z - niepoprawny login 
    if(data1 == "1"): 
        print("Użytkownik zalogowal sie")
        Output.delete('1.0', END)
        Output.insert(END, ("Użytkownik zalogowal sie\n\n"))
        zalogowany = 1
        global login
        login = data
        myLabel = Label(root, text=login)
        myLabel.pack()
    elif(data1 == "j"):
        print("proszę wybrać innego użytkownika")
        Output.delete('1.0', END)
        Output.insert(END, ("Proszę wybrać innego użytkownika\n\n"))
    elif(data1 == "z"):
        print("zły login")
        Output.delete('1.0', END)
        Output.insert(END, ("Zły login\n\n"))

def przycisk_wysylania(value): #pisanie wiadomości
    if(len(value)< 1):
        return
    global zalogowany
    if(zalogowany == 0):
        Output.delete('1.0', END)
        Output.insert(END, ("Nie jesteś zalogowany\n\n"))
        return
    INPUT = inputtxt.get("1.0", "end-1c")
    if(len(INPUT) < 1):
        return
    client.send(":send".encode(FORMAT))
    data = client.recv(SIZE).decode(FORMAT)
    client.send(value.encode(FORMAT))
    data = client.recv(SIZE).decode(FORMAT)
    if(data=="0"):
        Output.insert(END, ("Zły login\n\n"))
        return
    if(data=="ow"):
        Output.insert(END, ("Schowek "+value+ " jest przepelniony\n\n"))
        return
    client.send(INPUT[0:100].encode(FORMAT))
    data = client.recv(SIZE).decode(FORMAT)
    Output.insert(END, ("Wiadomość wysłana do "+value+ ":\n"+INPUT[0:100]+"\n\n"))

def przycisk_odbierania(): #odbieranie wiadomości
    global zalogowany
    if(zalogowany == 0):
        return
    data = ":recv"
    client.send(data.encode(FORMAT))
    BigData = client.recv(128).decode(FORMAT)
    if(len(BigData) <= 1):
        client.send("end".encode(FORMAT))
        BigData = client.recv(128).decode(FORMAT)
        return
    Output.insert(END, BigData[1:])
    client.send("check".encode(FORMAT))
    while(True):
        BigData = client.recv(128).decode(FORMAT)
        if(BigData == "false"):
            Output.insert(END, "")
            break
        client.send("check".encode(FORMAT))
        BigData = client.recv(128).decode(FORMAT)
        Output.insert(END, BigData)
        client.send("check".encode(FORMAT))

x = threading.Thread(target=odbieranie, args=())
x.start()


myButton2 = Button(root, text="logowanie", command=lambda: przycisk_logowania(user.get()))
myButton2.pack()
myButton = Button(root, text="send",command=lambda: przycisk_wysylania(user.get()))
myButton.pack()

inputtxt = Text(root, height = 1,
                width = 100,
                bg = "light yellow")
  
Output = Text(root, height = 15, 
                width = 100, 
                bg = "light cyan")
  
inputtxt.pack()
Output.pack()
mainloop()
exit = 1
x.join()
data = ":exit"
client.send(data.encode(FORMAT))
client.close()
print(":exit")