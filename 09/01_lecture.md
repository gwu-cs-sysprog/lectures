
# System Services, Daemons, and Bootup

The `systemctl` command enables you to understand many of the services on the system (of which there are many: `systemctl --list-units | wc -l` yields `244` on my system).

```
$ systemctl --list-units
...
  cups.service           loaded active     running   CUPS Scheduler
...
  gdm.service            loaded active     running   GNOME Display Manager
...
  ssh.service            loaded active     running   OpenBSD Secure Shell server
...
  NetworkManager.service loaded active     running   Network Manager
...
  openvpn.service        loaded active     exited    OpenVPN service
...
```

I've pulled out a few selections that are easier to relate to:

- CUPS which is the service used to receive print jobs.
- [GDM](https://en.wikipedia.org/wiki/GNOME_Display_Manager) which manages your *graphical logins* on Ubuntu.
- OpenSSH which manages your *remote* logins through `ssh`.
- NetworkManager that you interact with when you select which wifi hotspot to use.
- OpenVPN which handles your VPN logins to the school network.
    You can see that the service is currently "exited" as I'm not running VPN now.

We'll cover *services* later in the class, and here we'll focus mainly on libraries.
