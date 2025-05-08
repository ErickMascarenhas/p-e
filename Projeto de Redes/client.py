import socket
import threading
from os import system
import time

HOST = 'localhost'
PORT = 5555

def recebermsgs(cliente):
    while True:
        try:
            msg = cliente.recv(2048).decode()
            if not msg:
                break
            if "saiu do jogo." not in msg:
                system("cls")
            print(msg)
            if "segundos" in msg: # se a rodada terminou, espera 5 segundos para iniciar a próxima
                time.sleep(5)

        except:
            print("[ERRO] Conexão encerrada.")
            break

def main():
    cliente = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        cliente.connect((HOST, PORT))
    except:
        print("Não foi possível se conectar ao servidor.")
        return

    thread = threading.Thread(target=recebermsgs, args=(cliente,))
    thread.start()

    while True:
        try:
            msg = input()
            if msg.strip().lower() == "sair":
                break
            cliente.sendall(msg.encode())
        except:
            break

    cliente.close()

main()