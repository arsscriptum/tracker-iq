<center><img src="doc/img/tracker_rank_banner.png" alt="banner"></center>

# dht server

My own custom DHT server using libtorrent-rasterbar


This program will
✔ Enable DHT mode  
✔ Set up DHT bootstrap nodes  
✔ Listen on UDP port 6881  
✔ Print DHT alerts (log messages)


### How It Works

✅ DHT is enabled and starts listening on UDP port 6881.  
✅ It receives peer requests (DHT announce & reply messages).  
✅ It logs which torrents peers are sharing.  
✅ Acts as a DHT bootstrap node for any client that connects.

### How to Use This DHT Server
- Modify qBittorrent or any BitTorrent client to use your custom DHT node:
  ```
  udp://your-dht-server-ip:6881
  ```
- Run multiple instances across different servers to create redundant bootstrap nodes.

### Enhancing the DHT Server
- Save discovered peers in an SQLite database.
- Run on multiple servers to improve peer discovery.
- Filter logs to only show unique IPs and torrents.
- Add an HTTP API to allow users to query active peers.

When working with DHT (Distributed Hash Table) in libtorrent, you will encounter different types of alerts that provide information about how peers are interacting with the DHT network. Two important ones are `dht_reply_alert` and `dht_announce_alert`.



## Technical Details

## 1. What is `dht_reply_alert`?
### When do you receive a `dht_reply_alert`?
A `dht_reply_alert` is triggered when your BitTorrent client sends a "get_peers" request to the DHT network to find peers for a specific torrent info_hash, and some nodes reply with peer information.

### What does it mean?
- It means some DHT nodes responded with peers that are currently sharing the torrent.
- These peers may be seeders (uploading) or leechers (downloading) the requested torrent.

### What does it contain?
| Field            | Description |
|--||
| `info_hash`      | The torrent identifier (hash of the metadata). |
| `num_peers`      | How many peers were returned from the DHT query. |
| `tracker_url()`  | The DHT node or tracker that returned the peers. |

### Example Scenario
1. You add a magnet link in your torrent client.
2. Your client sends a `get_peers` request to DHT nodes, asking "Who has this torrent?".
3. Some DHT nodes respond with peers that have the file.
4. `dht_reply_alert` is triggered, containing the list of discovered peers.

### What can you do with it?
- Store peer count per `info_hash` in a database.
- Display how many peers were found.
- Log which DHT node provided the response.



## 2. What is `dht_announce_alert`?
### When do you receive a `dht_announce_alert`?
A `dht_announce_alert` is triggered when another peer announces itself on the DHT network, saying:
> *"I have this torrent, and I am available for downloading!"*

### What does it mean?
- A new seeder or leecher has joined the swarm for a specific torrent.
- Your client may connect to this peer for downloading.

### What does it contain?
| Field         | Description |
|--||
| `info_hash`  | The torrent identifier (same as `dht_reply_alert`). |
| `ip`         | The IP address of the announcing peer. |

### Example Scenario
1. A new seeder starts sharing a torrent on the DHT network.
2. They announce themselves using `put_immutable_item` in DHT.
3. Your client receives a `dht_announce_alert`, telling you:
   > *"A peer at `192.168.1.100` is sharing this torrent!"*

### What can you do with it?
- Log and store the IP of announcing peers.
- Prioritize fresh peers for faster downloads.
- Detect new swarm activity (if new peers are appearing).



## 3. Difference Between `dht_reply_alert` and `dht_announce_alert`
| Feature           | `dht_reply_alert` | `dht_announce_alert` |
||||
| Triggered When? | You request peers from DHT. | A peer announces itself on DHT. |
| What Does It Tell You? | DHT nodes returned known peers for a torrent. | A new peer joined the swarm. |
| Key Fields? | `info_hash`, `num_peers`, `tracker_url()` | `info_hash`, `ip` of peer |
| Use Case? | Find existing peers in DHT. | Find new peers in real-time. |



## 4. How to Use These Alerts Effectively
- Use `dht_reply_alert` to collect a list of available peers before starting a download.
- Use `dht_announce_alert` to dynamically add new peers to improve torrent speed.
- Log `info_hash` occurrences to detect popular torrents.



### 5. Example Console Output
If you log both alerts:
```
[ DHT REPLY ] InfoHash=abc123456789def, Peers=5, Tracker=router.bittorrent.com
[ DHT ANNOUNCE ] InfoHash=abc123456789def, Peer=192.168.1.100
```

This tells us:
- 🟢 We found 5 peers from DHT for `abc123456789def`.
- 🟢 A new peer at 192.168.1.100 just joined the swarm for the same torrent.




## How to Build

### Visual Studio

1. Clone [tracker-iq](https://github.com/arsscriptum/tracker-iq) recursively

```bash 
git clone --recurse-submodules https://github.com/arsscriptum/tracker-iq.git
```

2. Build Dependencies
```powershell
./scripts/MakeDeps.ps1
```

3. Open and build ```tracker-iq.vcxproj```

```powershell
./Build.ps1
```

<center><img src="doc/img/deps_all.gif" alt="banner3"></center>

