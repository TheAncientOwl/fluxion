# Only one run / each version+file_size

### [@] Git commit: `b0dfbb3db898a4d8423e1152fcc5569430696286`

### [?] Difference between `V4` & `V3`: `V4 uses mmap + count lines parallel`

---

# 1. ~ 7mb file

### 1.1. V4 ~ 7mb

```
| 19:33:25:656:992000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs » [-] End   » ::CountLinesParallel() ~ elapsed 301417ns
| 19:33:25:717:180000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs » [-] End   » ::ImportLogs(): writer ~ elapsed 55ms, 929667ns
| 19:33:25:719:004000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs » [-] End   » ::ImportLogs() ~ elapsed 66ms, 616375ns
```

### 1.2. V3 ~ 7mb

```
| 19:35:28:597:763000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs » [-] End   » ::CountLines() ~ elapsed 5ms, 796917ns
| 19:35:28:671:823000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs » [-] End   » ::ImportLogs(): writer ~ elapsed 71ms, 769166ns
| 19:35:28:672:046000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs » [-] End   » ::ImportLogs() ~ elapsed 80ms, 94584ns
```

---

# 2. ~ 650mb file

### 2.1. V4 ~ 650mb

```
| 19:33:35:547:920000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs » [-] End   » ::CountLinesParallel() ~ elapsed 9ms, 89458ns
| 19:33:40:211:829000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs » [-] End   » ::ImportLogs(): writer ~ elapsed 4s, 657ms, 605250ns
| 19:33:40:216:061000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs » [-] End   » ::ImportLogs() ~ elapsed 4s, 998ms, 70917ns
```

### 2.2. V3 ~ 650mb

```
| 19:35:44:611:422000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs » [-] End   » ::CountLines() ~ elapsed 194ms, 440458ns
| 19:35:49:565:044000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs » [-] End   » ::ImportLogs(): writer ~ elapsed 4s, 940ms, 851875ns
| 19:35:49:565:253000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs » [-] End   » ::ImportLogs() ~ elapsed 5s, 148ms, 310292ns
```

---

# 3. ~ 3gb file

### 3.1. V4 ~ 3gb

```
| 19:33:46:788:425000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs » [-] End   » ::CountLinesParallel() ~ elapsed 69ms, 125ns
| 19:34:09:797:787000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs » [-] End   » ::ImportLogs(): writer ~ elapsed 22s, 997ms, 793833ns
| 19:34:09:812:843000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs » [-] End   » ::ImportLogs() ~ elapsed 23s, 202ms, 513667ns
```

### 3.2. V3 ~ 3gb

```
| 19:35:56:205:754000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs » [-] End   » ::CountLines() ~ elapsed 963ms, 656042ns
| 19:36:21:136:904000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs » [-] End   » ::ImportLogs(): writer ~ elapsed 24s, 878ms, 918167ns
| 19:36:21:137:080000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs » [-] End   » ::ImportLogs() ~ elapsed 25s, 895ms, 132083ns
```

---

# 4. ~ 6gb file

### 4.1. V4 ~ 6gb

```
| 19:34:30:895:083000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs » [-] End   » ::CountLinesParallel() ~ elapsed 759ms, 212583ns
| 19:35:18:127:600000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs » [-] End   » ::ImportLogs(): writer ~ elapsed 47s, 204ms, 828208ns
| 19:35:18:153:253000 |    scope | Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs » [-] End   » ::ImportLogs() ~ elapsed 51s, 1ms, 808792ns
```

### 4.2. V3 ~ 6gb

```
| 19:36:30:797:372000 | scope | Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs » [-] End » ::CountLines() ~ elapsed 1s, 772ms, 862083ns
| 19:37:22:109:141000 | scope | Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs » [-] End » ::ImportLogs(): writer ~ elapsed 51s, 299ms, 378542ns
| 19:37:22:109:800000 | scope | Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs » [-] End » ::ImportLogs() ~ elapsed 53s, 85ms, 587291ns

```

---
