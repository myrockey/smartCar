#include "stm32f10x.h"
#include "stdio.h"
#include <string.h>
#include "RingBuff.h"

RingBuff_t encoeanBuff;

void RingBuff_Init(RingBuff_t *rb) //��ʼ������
{
  rb->Head = 0; //ͷָ��������ʼλ
  rb->Tail = 0; //βָ��������ʼλ
  rb->Length = 0; //��¼��ǰ���ݳ��� �ж��Ƿ��������
  //memset(rb->Ring_Buff,0,sizeof(rb->Ring_Buff));
}

/**
 * @brief  �ж϶����Ƿ�Ϊ��
 * @note   
 * @param  *rb: �ṹ��ָ��
 * @retval ����0��1,1����գ�0����ǿ�
 */
te_cicrleQueueStatus_t RingBuff_IsEmpty(RingBuff_t *rb)
{
  return (rb->Head == rb->Tail) ? CQ_STATUS_IS_EMPTY : CQ_STATUS_OK;
}
/**
 * @brief  �ж϶����Ƿ�Ϊ��
 * @note   
 * @param  *rb: �ṹ��ָ��
 * @retval ����0��1,1��������0�������
 */
te_cicrleQueueStatus_t RingBuff_IsFull(RingBuff_t *rb)
{
  return ((rb->Tail + 1) % BUFFER_SIZE == rb->Head) ? CQ_STATUS_IS_FULL : CQ_STATUS_OK;
}

/**
  *���ܣ�����д�뻷�λ�����
  *���1��Ҫд�������
  *���2��bufferָ��
  *����ֵ��buffer�Ƿ�����
  */
uint8_t Write_RingBuff(RingBuff_t *ringBuff , uint8_t data)
{
    //�����ֽ����ݴ��뵽����buffer��tailβ��
	ringBuff->Ring_Buff[ringBuff->Tail]=data;    
    //����ָ������buffer��β����ַ����ֹԽ��Ƿ�����
	ringBuff->Tail = ( ringBuff->Tail + 1 ) % BUFFER_SIZE;
    //����һ���ֽ����ݳɹ���len��1 
	ringBuff->Length++;    
	if(ringBuff->Length >= BUFFER_SIZE) //�жϻ������Ƿ�����
	{
		//���buffer�����ˣ����buffer���������³�ʼ��   ����ʼ�����Ḵλ����
		// memset(ringBuff, 0, BUFFER_SIZE);
		// RingBuff_Init(&ringBuff);
		printf("����������\r\n");
		ringBuff->Length = BUFFER_SIZE;
		ringBuff->Head = ( ringBuff->Tail + 1 ) % BUFFER_SIZE;
		//return 1;
	}
	return 0;
}

/**
  *���ܣ���ȡ��������֡����-���ֽڶ�ȡ
  *���1�������ȡ���ݵ�ָ��
  *���2��������bufferָ��
  *����ֵ���Ƿ�ɹ���ȡ����
  */
uint8_t Read_RingBuff_Byte(RingBuff_t *ringBuff , uint8_t *rData)
{
	if(ringBuff->Length == 0)//�жϷǿ�
	{
		return 1;
	}
		
    //�Ƚ��ȳ�FIFO���ӻ�����ͷ������ͷλ������ȡ��
	*rData = ringBuff->Ring_Buff[ringBuff->Head];
    //��ȡ�����ݵ�λ�ã���������
	ringBuff->Ring_Buff[ringBuff->Head] = 0;
				
	//����ָ��bufferͷ��λ�ã���ֹԽ��Ƿ�����
	ringBuff->Head = (ringBuff->Head + 1) % BUFFER_SIZE;
    //ȡ��һ���ֽ����ݺ󣬽����ݳ��ȼ�1
	ringBuff->Length--;
	
	return 0;
}


/*
�ӻ��λ�����������ֽ�
*/
te_cicrleQueueStatus_t RingBuff_ReadNByte(RingBuff_t *pRingBuff, uint8_t *pData, int size)
{
	int i = 0;
	if(NULL == pRingBuff || NULL == pData)
		return CQ_STATUS_ERR;

	for( i = 0; i < size; i++)
	{
		Read_RingBuff_Byte(pRingBuff, pData+i);
	}
	return CQ_STATUS_OK;
}

//���λ�����д����ֽ�
te_cicrleQueueStatus_t RingBuff_WriteNByte(RingBuff_t *pRingBuff, uint8_t *pData, int size)
{
	int i = 0;
	if(NULL == pRingBuff || NULL == pData)
		return CQ_STATUS_ERR;

	for(i = 0; i < size; i++)
	{
		Write_RingBuff(pRingBuff, *(pData+i));
	}
	return CQ_STATUS_OK;
}


//��ȡ��ǰ���λ����������ݳ���
int RingBuff_GetLen(RingBuff_t *pRingBuff)
{
	if(NULL == pRingBuff)
		return 0;

	if(pRingBuff->Tail >= pRingBuff->Head)
	{
		return pRingBuff->Tail - pRingBuff->Head;
	}
	
	return pRingBuff->Tail + BUFFER_SIZE - pRingBuff->Head;
}

uint16_t RQBuff_GetBuffLenth(RingBuff_t* RQ_Buff) {
	return RQ_Buff->Length;
}

//��ȡ��ǰͷ������
unsigned char RingBuff_GetHeadItem(RingBuff_t *pRingBuff)
{
	if(NULL == pRingBuff)
		return CQ_STATUS_ERR;
	
	return pRingBuff->Ring_Buff[pRingBuff->Head];
}

//��ȡָ���±�����
unsigned char RingBuff_GetIndexItem(RingBuff_t *pRingBuff, int index)
{
	if(NULL == pRingBuff || index > BUFFER_SIZE-1)
		return CQ_STATUS_ERR;

	return pRingBuff->Ring_Buff[index%BUFFER_SIZE];
}

