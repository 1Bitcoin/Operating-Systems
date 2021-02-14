#include <stdio.h>
#include <windows.h>

#define _CRT_SECURE_NO_WARNINGS
#define COUNT_READERS 3
#define COUNT_WRITERS 2

struct handle_information
{
	LONG active_readers = 0;
	LONG waiting_readers = 0;
	LONG waiting_writers = 0;
	bool active_writer = false;

	HANDLE mutex;
	HANDLE can_read;
	HANDLE can_write;

	int *buffer;
};

void start_write(handle_information* my_handle)
{
	InterlockedIncrement(&my_handle->waiting_writers);

	// обеспечение монопольного доступа писателя
	if (my_handle->active_writer || my_handle->active_readers > 0)
	{
		WaitForSingleObject(my_handle->can_write, INFINITE);
	}

	WaitForSingleObject(my_handle->mutex, INFINITE);
	InterlockedDecrement(&my_handle->waiting_writers);
	my_handle->active_writer = true;

	// сброс "в ручную"
	ResetEvent(my_handle->can_write);

	ReleaseMutex(my_handle->mutex);
}

void start_read(handle_information* my_handle)
{
	InterlockedIncrement(&my_handle->waiting_readers);
	if (my_handle->active_writer || my_handle->waiting_writers > 0)
	{
		WaitForSingleObject(my_handle->can_read, INFINITE);
	}

	InterlockedDecrement(&my_handle->waiting_readers);
	InterlockedIncrement(&my_handle->active_readers);

	// чтобы следующий читатель в очереди читателей смог начать чтение
	SetEvent(my_handle->can_read);
}

void stop_write(handle_information* my_handle)
{
	my_handle->active_writer = false;
	if (WaitForSingleObject(my_handle->can_read, 0) == WAIT_OBJECT_0)
	{
		SetEvent(my_handle->can_read);
	}
	else
	{
		SetEvent(my_handle->can_write);
	}
}

void stop_read(handle_information* my_handle)
{
	// уменьшение количества активных писаталей
	InterlockedDecrement(&my_handle->active_readers);
	if (my_handle->active_readers == 0)
	{
		// активизация писателя из очереди писателей
		SetEvent(my_handle->can_write);
	}
}

DWORD reader(handle_information* my_handle)
{
	while (true)
	{
		start_read(my_handle);
		printf("Reader #%ld read <- %d\n", GetCurrentThreadId(), *my_handle->buffer);
		stop_read(my_handle);

		Sleep(1000 * (rand() % 3));
	}

	return 0;
}

DWORD writer(handle_information* my_handle)
{
	while (true)
	{
		start_write(my_handle);
		printf("Writer #%ld write -> %ld\n", GetCurrentThreadId(), ++(*my_handle->buffer));
		stop_write(my_handle);

		Sleep(900 * (rand() % 3));
	}

	return 0;
}

handle_information* Initialization(int *buffer)
{
	// создает новый своободный мьютекс
	HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
	if (mutex == NULL)
	{
		perror("Can't create mutex");
		exit(1);
	}

	// создание Event, который переключается "в ручную", объект в сигнальном состоянии
	HANDLE can_write = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (can_write == NULL)
	{
		perror("Can't create event can write");
		exit(1);

	}

	// создание Event, который переключается автоматически, объект в сигнальном состоянии
	HANDLE can_read = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (can_read == NULL)
	{
		perror("Can't create event can read");
		exit(1);

	}

	handle_information* my_handle = new handle_information();

	my_handle->mutex = mutex;
	my_handle->can_read = can_read;
	my_handle->can_write = can_write;
	my_handle->buffer = buffer;

	return my_handle;
}

int main()
{
	HANDLE writers[COUNT_WRITERS];
	HANDLE readers[COUNT_READERS];

	int buffer = 0;

	handle_information* my_handle = Initialization(&buffer);

	for (int i = 0; i < COUNT_WRITERS; i++)
	{
		writers[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&writer, my_handle, 0, NULL);
		if (writers[i] == NULL)
		{
			perror("Can't create writer");
			return 1;
		}
	}

	for (int i = 0; i < COUNT_READERS; i++)
	{
		readers[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&reader, my_handle, 0, NULL);
		if (readers[i] == NULL)
		{
			perror("Can't create reader");
			return 1;
		}
	}

	// ожидание освобождения
	WaitForMultipleObjects(COUNT_WRITERS, writers, TRUE, INFINITE);
	WaitForMultipleObjects(COUNT_READERS, readers, TRUE, INFINITE);

	// освобождение ресурсов
	CloseHandle(my_handle->mutex);
	CloseHandle(my_handle->can_read);
	CloseHandle(my_handle->can_write);

	return 0;
}
