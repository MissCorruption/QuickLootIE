# General

### Behavior

| Name                            | Type   | Default |
|---------------------------------|--------|---------|
| Show in combat                  | `bool` | `true`  |
| Show when container is empty    | `bool` | `false` |
| Show when container is unlocked | `bool` | `true`  |
| Show in third person            | `bool` | `true`  |
| Show when mounted               | `bool` | `false` |
| Enable for corpses              | `bool` | `true`  |
| Enable for animals              | `bool` | `true`  |
| Enable for dragons              | `bool` | `true`  |
| Break invisibility when used    | `bool` | `true`  |
| Play scroll sounds              | `bool` | `true`  |

# Display

### Window Settings

| Name                            | Type                    | Default    |
|---------------------------------|-------------------------|------------|
| Window anchor                   | `enum`                  | `Top Left` |
| Window offset X                 | `int`   [`-960`; `960`] | `100`      |
| Window offset Y                 | `int`   [`-540`; `540`] | `-200`     |
| Window scale                    | `float` [`0.1`; `3.0`]  | `1.0`      |
| Window opacity                  | `float` [`0.1`; `1.0`]  | `1.0`   |
| Window opacity when empty       | `float` [`0.1`; `1.0`]  | `0.3`   |
| Minimum number of lines to show | `int`   [`0`; `25`]     | `0`     |
| Maximum number of lines to show | `int`   [`1`; `25`]     | `7`     |

> [!Note]  
> Valid values for `Anchor` are  `Top Left`, `Center Left`, `Bottom Left`, `Top Center`, `Center`, `Bottom Center`, `Top Right`, `Center Right` and `Bottom Right`.

> [!Note]  
> When setting Max Lines to a value smaller than Min Lines, Min Lines should be updated to take on the same value.  
> When setting Min Lines to a value larger than Max Lines, Max Lines should be updated to take on the same value.

### Icons

| Name                            | Type   | Default |
|---------------------------------|--------|---------|
| Show the item icon              | `bool` | `true`  |
| Show book read icon             | `bool` | `true`  |
| Show stolen icon                | `bool` | `true`  |
| Show item enchanted icon        | `bool` | `true`  |
| Show known enchantment icon     | `bool` | `true`  |
| Show special enchantment icon   | `bool` | `true`  |

# Sorting

### By Item Category

- Gold
- Gems
- Soul Gems
- Lockpicks
- Ingots
- Ores
- Potions
- Food & Drinks
- Books
- Notes
- Scrolls
- Arrows & Bolts
- Jewelry
- Weapons
- Armors
- Clothes

### By Attribute

- Weightless
- By Weight
- By Value
- By Name
- LOTD Needed
- Completionist Needed

# Controls

### Keybindings

| Name                            | Type   | Default |
|---------------------------------|--------|---------|
| Take                            | `key`  | `E`     |
| Take All                        | `key`  | `R`     |
| Search                          | `key`  | `Q`     |
| Disable loot menu               | `key`  | unbound |
| Enable loot menu                | `key`  | unbound |

# Compatibility

### Artifact Icons

| Name                            | Type   | Default |
|---------------------------------|--------|---------|
| Show new artifact icon          | `bool` | `true`  |
| Show carried artifact icon      | `bool` | `true`  |
| Show displayed artifact icon    | `bool` | `true`  |

### Completionist

| Name                            | Type   | Default |
|---------------------------------|--------|---------|
| Show needed item icon           | `bool` | `true`  |
| Show collected item icon        | `bool` | `true`  |
