from tkinter import *
import os
import socket
import sys
import threading
import time

IP = sys.argv[1]
PORT = 4444
ADDR = (IP, PORT)
SIZE = 128
FORMAT = "utf-8"

# łączenie z serwerem
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(ADDR)


# Logowanie
def login():
	global login_screen
	login_screen = Tk()
	login_screen.title("GG")

	# rozmiar i pozycja okna
	w, h = 400, 450
	global ws , hs
	ws = login_screen.winfo_screenwidth() # rozmiary ekranu
	hs = login_screen.winfo_screenheight()
	x = (ws/2) - (w/2)
	y = (hs/2) - (h/2)
	login_screen.geometry('%dx%d+%d+%d' % (w, h, x, y))

	Label(text="Welcome to the chat!\nPlease enter your login and password below!", bg="gray", width=w, height="4", font=("Calibri", 12)).pack()
	Label(text="\n\n\n").pack()
 
	global username_verify
	global password_verify
 
	username_verify = StringVar()
	password_verify = StringVar()
 
	global username_login_entry
	global password_login_entry
 
	Label(login_screen, text="Username").pack()
	username_login_entry = Entry(login_screen, textvariable=username_verify)
	username_login_entry.pack()
	Label(login_screen, text="").pack()
	Label(login_screen, text="Password").pack()
	password_login_entry = Entry(login_screen, textvariable=password_verify, show= '*')
	password_login_entry.pack()
	Label(login_screen, text="").pack()
	Button(login_screen, text="Login", width=10, height=1, command = login_verify).pack()
	
	login_screen.mainloop()


# Weryfikacja logowania
def login_verify():
	global username1
	username1 = username_verify.get()
	password1 = password_verify.get()
	
	if (username1 == "" or password1 == ""):
		return
	
	username_login_entry.delete(0, END)
	password_login_entry.delete(0, END)

	client.send(username1.encode(FORMAT)) # wysyła login do servera
	data1 = client.recv(SIZE).decode(FORMAT)
	# odbiera wynik logowania
	if(data1 == "pl"): # poprawny login
		client.send(password1.encode(FORMAT)) # wysyła hasło do servera
		data2 = client.recv(SIZE).decode(FORMAT)
		if (data2 == "ph"): # poprawne hasło
			print("Logged in as "+username1+".")
			main_account()
		elif (data2 == "nh"): # niepoprawne hasło
			password_not_recognised()
	elif(data1 == "j"): # ktoś inny już jest zalogowany pod tym samym loginem
		user_already_logged_in()
	elif(data1 == "nl"): # niepoprawny login
		user_not_found()


# Komunikat - niepoprawne hasło
def password_not_recognised():
	global password_not_recog_screen
	password_not_recog_screen = Toplevel(login_screen)
	password_not_recog_screen.title("GG Login")
	
	# rozmiar i pozycja okna
	w, h = 200, 100
	x = (ws/2) - (w/2)
	y = (hs/2) - (h/2)
	password_not_recog_screen.geometry('%dx%d+%d+%d' % (w, h, x, y))
	
	Label(password_not_recog_screen, text="\nInvalid Password\n").pack()
	Button(password_not_recog_screen, text="OK", command=delete_password_not_recognised).pack()


# Komunikat - nie znaleziono użytkownika
def user_not_found():
	global user_not_found_screen
	user_not_found_screen = Toplevel(login_screen)
	user_not_found_screen.title("GG Login")
	
	# rozmiar i pozycja okna
	w, h = 200, 100
	x = (ws/2) - (w/2)
	y = (hs/2) - (h/2)
	user_not_found_screen.geometry('%dx%d+%d+%d' % (w, h, x, y))
	
	Label(user_not_found_screen, text="\nUser Not Found\n").pack()
	Button(user_not_found_screen, text="OK", command=delete_user_not_found_screen).pack()
	
	
# Komunikat - ktoś jest już zalogowany na tym koncie
def user_already_logged_in():
	global user_already_logged_in_screen
	user_already_logged_in_screen = Toplevel(login_screen)
	user_already_logged_in_screen.title("GG Login")
	
	# rozmiar i pozycja okna
	w, h = 250, 150
	x = (ws/2) - (w/2)
	y = (hs/2) - (h/2)
	user_already_logged_in_screen.geometry('%dx%d+%d+%d' % (w, h, x, y))
	
	Label(user_already_logged_in_screen, text="\nSomeone is already logged in\nto this account.\n").pack()
	Button(user_already_logged_in_screen, text="OK", command=delete_user_already_logged_in_screen).pack()


def delete_login():
	login_screen.destroy()
 
 
def delete_password_not_recognised():
	password_not_recog_screen.destroy()
 
 
def delete_user_not_found_screen():
	user_not_found_screen.destroy()
	
	
def delete_user_already_logged_in_screen():
	user_already_logged_in_screen.destroy()


# Pisanie wiadomości
def przycisk_wysylania(receiver):
	INPUT = Input.get("1.0", "end-1c")
	if(len(INPUT) < 1):
		return
	client.send(":send".encode(FORMAT))
	data = client.recv(SIZE).decode(FORMAT)
	client.send(receiver.encode(FORMAT))
	data = client.recv(SIZE).decode(FORMAT)
	Output.configure(state='normal')
	if(data=="0"):
		Output.insert(END, ("Zły login\n\n"))
		return
	if(data=="ow"):
		Output.insert(END, ("Schowek "+receiver+ " jest przepelniony\n\n"))
		return
	client.send(INPUT[0:100].encode(FORMAT))
	data = client.recv(SIZE).decode(FORMAT)
	Output.insert(END, ("Wiadomość wysłana do "+receiver+ ":\n"+INPUT[0:100]+"\n\n"))
	Output.configure(state='disabled')
	Input.delete(1.0, END)


# Odbieranie wiadomości
def odbieranie():
	while(exit == 0):
		time.sleep(0.3)
		data = ":recv"
		client.send(data.encode(FORMAT))
		BigData = client.recv(128).decode(FORMAT)
		if(len(BigData) <= 1):
			client.send("end".encode(FORMAT))
			BigData = client.recv(128).decode(FORMAT)
		else:
			Output.configure(state='normal')
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
			Output.configure(state='disabled')



# Ekran główny - chat
def main_account():
	global exit
	exit = 0
	delete_login()
	global main_account_screen
	main_account_screen = Tk()
	main_account_screen.title("GG")
	
	# rozmiar i pozycja okna
	w, h = 500, 500
	x = (ws/2) - (w/2)
	y = (hs/2) - (h/2)
	main_account_screen.geometry('%dx%d+%d+%d' % (w, h, x, y))
	
	# wątek do odbierania wiadomości
	odb = threading.Thread(target=odbieranie, args=())
	odb.start()

	Label(main_account_screen, text="Welcome, "+username1+"!", 
	      font=("Calibri", 13), bg="gray", width=w, height=2).pack()
	Label(main_account_screen, text="\n\n").pack()
	
	# wczytanie listy użytkowników z pliku userFriends
	loginArray = []
	file1 = open('userFriends', 'r')
	Lines = file1.readlines()
	for line in Lines:
		loginArray.append(line.strip())
	file1.close()
		
	# wybieranie odbiorcy wiadomości - rozwijana lista
	receiver = StringVar(main_account_screen)
	receiver.set(loginArray[0])
	options = OptionMenu(main_account_screen, receiver, *loginArray)
	options.config(width=90, font=('Helvetica', 12))
	options.pack(side="top")
	mess = Label(text="Send a message to {}".format(loginArray[0]), font=('Helvetica', 12), fg='gray')
	mess.pack(side="top")
	def callback(*args):
		mess.configure(text="Send a message to {}".format(receiver.get()))
	receiver.trace("w", callback)

	global Output
	Output = Text(main_account_screen, height = 15, width = 100, bg = "white")
	Output.configure(state='disabled')
	Output.pack()
	global Input
	Input = Text(main_account_screen, height = 2, width = 100, bg = "white")
	Input.pack()
	sendButton = Button(main_account_screen, text="send", command=lambda: przycisk_wysylania(receiver.get()))
	sendButton.pack()

	main_account_screen.mainloop()
	exit = 1
	odb.join()



login()

client.send(":exit".encode(FORMAT))
client.close()


