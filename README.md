Everzone

Everzone is a multiplayer third-person shooter built using Unreal Engine. It is based on a listen/server model, offering fast-paced action and strategic gameplay with an emphasis on networking and replication.

Features

Gameplay Modes:
Free for All (FFA): Every player for themselves. The player with the highest score wins.

Team Deathmatch (TDM): Two teams compete to reach the score limit by eliminating the opposing team.

Capture the Flag (CTF): Teams capture the enemy flag and bring it back to their base while defending their own flag.

Weapon System:
There are three types of weapons Projectile Weapons that fire a projectile based class, Hitscan Weapons that perform single line traces(raycast) and Shotgun Weapon which fires multi line traces
A diverse arsenal to choose from, including:

Pistols (Hitscan)

Submachine Guns (SMGs) (Hitscan)

Sniper Rifles (Hitscan)

Assault Rifles (Projectile)

Shotguns (Shotgun)

Rocket Launchers (Hitscan)

Grenade Launchers (Hitscan)

Power-Up System:
Gain the upper hand in battles with various power-ups:

Health Packs: Restore health.

Shields: Temporarily absorb damage.

Speed Boost: Move faster for a limited time.

Jump Boost: Increase jump height for better mobility.

Grenade Supply: Replenish your grenades.

Networking & Replication:
Built with Unreal Engine's listen/server model.

RPCs (Remote Procedure Calls) functions are categorized and marked with either Server, Multicast or Client, ie. ServerFunction, MulticastFunction, ClientFunction:

Server RPCs: Executed on the server.

Multicast RPCs: Executed on all clients.

Client RPCs: Executed on a specific client.

Replicated variables are managed using the GetLifetimeReplicatedProps function with the DOREPLIFETIME macro.

Replication Notify Functions (e.g., On_Rep functions): Triggered when replicated variables are updated.


How to Play:
Launch the game.

To host a session:

Select the game mode.

Enter the number of playtesters.

Press Host to create the session.

If you are not hosting:

Wait for the host to create the session.

Press Join to enter the session.

Controls

W/A/S/D: Movement

Left Mouse Button: Fire weapon

Right Mouse Button: Aim down sights

Spacebar: Jump

E: Interact with objects

Q: Throw grenades

R: Reload

Escape: Quit

Technical Overview

Networking:
Fully replicated gameplay mechanics.

Extensive use of Unreal's networking system for smooth multiplayer experiences.

Game Systems:
Modular design for adding new weapons, power-ups, and game modes.
