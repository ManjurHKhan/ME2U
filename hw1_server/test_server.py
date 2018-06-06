import socket
import threading

def ending(string, to_do = "add"):
    ending = "\r\n\r\n"
    if to_do == "add":
        return string + ending
    elif to_do == "remove":
        return string.replace(ending, "")

class User():
    def __init__(self, name, conn, addr):
        self.name = name
        self.conn = conn
        self.addr = addr

threads = []
users = []
def listen():
    global threads

    connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connection.bind(('0.0.0.0', 5555))
    
    while True:
        connection.listen(10)
    
        conn, addr = connection.accept()
        t = threading.Thread(target=handle_user, args=(conn, addr))
        t.daemon = True
        t.start()
        # threads.append(t)
        # t.start()
    # while 1:
        # try:
        # except Exception:

    connection.close()
        # this is accepting connection and calling threads

def handle_user(conn, addr):
    global users
    print("Connected by ", addr)
    while 1:
        data = conn.recv(4096)
        if not data:
            break
        print(data);
        new_str = ending(data.decode('utf-8'), "remove")

        if new_str == "ME2U":
            print(encode(ending("U2EM")))
            conn.send(encode(ending("U2EM")))
        elif new_str[:3] == "IAM":
            if check_user_exists(users, new_str[4:]):
                print(encode(ending("ETAKEN")))
                break;
            else:
                print(encode(ending("MAI")))
                conn.send(encode(ending("MAI")))
                print(encode(ending("MOTD Hello Human ")))
                conn.send(encode(ending("MOTD Hello Human ")))
                usr = User(new_str[4:], conn, addr)
                users.append(usr)
        elif new_str == "LISTU":
            temp = get_users(users)
            print(encode(ending("UTSIL " + temp)))
            conn.send(encode(ending("UTSIL " + temp)))
        elif new_str[:2] == "TO":
            (name, msg) = new_str[3:].split(' ', maxsplit=1)
            print(name, msg)
            if not find_user_to_send_msg_to(users, name, msg, get_name_from_addr(users, addr)):
                print(encode(ending("EDNE " + name)))
                conn.send(encode(ending("EDNE " + name)))
        elif new_str[:4] == "MORF":
            send_ot_to_usr(users, new_str[5:], get_name_from_addr(users, addr))
        elif new_str == "BYE":
            remove_usr_by_addr(users, addr)
        else:
            print("got value that I do not know about. closing.")
            conn.close()
        # print(ending("remove", data.decode('utf-8')).encode('utf-8'))

def send_ot_to_usr(users, name, from_name):
    for i in users:
        if i.name == name:
            print(encode(ending("OT " + from_name)))
            i.conn.send(encode(ending("OT " + from_name)))

def remove_usr_by_addr(users, addr):
    to_rem = None
    for i in users:
        if i.addr == addr:
            print(encode(ending("EYB")))
            i.conn.send(encode(ending("EYB")))
            to_rem = i
            break
    
    name = to_rem.name
    to_rem.conn.close()
    users.remove(to_rem)
    for i in users:
        print(encode(ending("UOFF " + name)))
        i.conn.send(encode(ending("UOFF " + name)))


def get_name_from_addr(users, addr):
    for i in users:
        if i.addr == addr:
            return i.name
    return None

def find_user_to_send_msg_to(users, name, msg, from_name):
    for i in users:
        if i.name == name:
            print(encode(ending("FROM " + from_name + " " + msg)))
            i.conn.send(encode(ending("FROM " + from_name + " " + msg)))
            return True
    return False
    

def get_users(users):
    if users is None:
        return ""

    to_ret = ""
    for usr in users:
        to_ret = to_ret + " " + usr.name
    return to_ret



def check_user_exists(users, name):
    if users is None:
        return False
    
    return any(x.name == name for x in users)


def encode(string):
    return string.encode('utf-8')

if __name__ == '__main__':
    try:
        print("Listenning on 127.0.0.1 at port 5555")
        listen()
    except KeyboardInterrupt:
        pass