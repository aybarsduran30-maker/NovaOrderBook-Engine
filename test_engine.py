import ctypes
import os
import sys

dll_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(dll_dir)

if sys.platform == "win32" and hasattr(os, "add_dll_directory"):
    os.add_dll_directory(dll_dir)
    os.add_dll_directory(r"C:\mingw64\bin")

dll_path = os.path.join(dll_dir, "nova_engine.dll")
engine = ctypes.CDLL(dll_path, winmode=0)

engine.create_order_book.restype = ctypes.c_void_p
engine.destroy_order_book.argtypes = [ctypes.c_void_p]

engine.add_order.argtypes = [ctypes.c_void_p, ctypes.c_uint64, ctypes.c_bool, ctypes.c_uint32, ctypes.c_uint32]
engine.add_order.restype = None

book = engine.create_order_book()

engine.add_order(book, 1, True, 100, 10)
print("Buy order processed.")

engine.add_order(book, 2, False, 100, 10)
print("Sell order processed.")

engine.destroy_order_book(book)
print("Cleaned up.")