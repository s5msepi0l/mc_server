from pycraft.networking.connection import Connection

conn = Connection('localhost', 25565)
conn.connect()
print("Connected to server!")
