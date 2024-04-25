# totally-not-rabbitmq
totally-not-rabbitmq

## Building (for Clion)

### 1. Clone rep
```
git clone 
```

### 2. Install conan
```
pip install conan
```

### 3. Install conan plugin in Clion
![Image alt](/media/plugin.png)


### 4. Configure Conan
![Image alt](/media/config.png)

#### Find path of Conan and paste in plugin configuration
```
where conan
```

![Image alt](/media/config2.png)

## Системный анализ базы данных

### Сущности
Менеджеры - выдает задания воркерам.
Поля:
- id
- name
- password
- email

Воркеры - выполняют задания.
Поля:
- id
- name
- password
- email

