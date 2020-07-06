from tkinter import *


def create_button_function():
    print("Create button pressed")


def join_button_function():
    print("Join button pressed")


root = Tk()
root.geometry("512x128")
root.title("PeersChat GUI Test")

left_frame = Frame(root)
left_frame.pack(side="left")
button = Button(left_frame,
                text="Create PeersChat Session",
                cursor="hand2",
                bd=4,
                height=4,
                width=28,
                relief="groove",
                command=create_button_function)
button.pack(padx=24)

right_frame = Frame(root)
right_frame.pack(side="right")
button2 = Button(right_frame,
                 text="Join PeersChat Session",
                 cursor="hand2",
                 bd=4,
                 height=4,
                 width=28,
                 relief="groove",
                 command=join_button_function)
button2.pack(padx=24)

root.mainloop()
