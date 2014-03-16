#include "MainWindow.h"
#include "Macro.h"

// Direct2D�̃I�u�W�F�N�g���g�����v�Z���y�ɂ��邽�߂ɉ��Z�q���I�[�o�[���[�h���Ă���
inline D2D1_POINT_2F operator+(D2D1_POINT_2F p1, D2D1_POINT_2F p2)
{
	p1.x += p2.x;
	p1.y += p2.y;
	return p1;
}
inline D2D1_POINT_2F operator-(D2D1_POINT_2F p1, D2D1_POINT_2F p2)
{
	p1.x -= p2.x;
	p1.y -= p2.y;
	return p1;
}
inline D2D1_POINT_2F &operator+=(D2D1_POINT_2F &p1, D2D1_POINT_2F p2)
{
	p1.x += p2.x;
	p1.y += p2.y;
	return p1;
}
inline D2D1_POINT_2F &operator-=(D2D1_POINT_2F &p1, D2D1_POINT_2F p2)
{
	p1.x -= p2.x;
	p1.y -= p2.y;
	return p1;
}
inline D2D1_POINT_2F operator*(D2D1_POINT_2F p, FLOAT f)
{
	p.x *= f;
	p.y *= f;
	return p;
}
inline D2D1_POINT_2F operator*(FLOAT f, D2D1_POINT_2F p)
{
	return p * f;
}
inline D2D1_POINT_2F operator/(D2D1_POINT_2F p, FLOAT f)
{
	p.x /= f;
	p.y /= f;
	return p;
}

unsigned int MainWindow::Run(void *ptr)
{
	auto _this = static_cast<MainWindow*>(ptr);

	LARGE_INTEGER freq, start, end;
	
	enum CounterType{
		SLEEP,
		PERFORMANCE
	}a = static_cast<CounterType>(::QueryPerformanceFrequency(&freq));

	if(a == PERFORMANCE)
		::QueryPerformanceCounter(&start);

	auto &pl = _this->player;
	auto &p_pos = _this->p_pos;
	auto &p_r = _this->data.p_r;
	auto &field = _this->data.field;

	D2D1_POINT_2F m_vec[2], p_vec = D2D1::Point2F();

	while(!_this->end){
		if(a == PERFORMANCE){
			while(::QueryPerformanceCounter(&end), 1000.f * (end.QuadPart - start.QuadPart) / freq.QuadPart < 15);
			start = end;
		}else
			::Sleep(15);

		// �}���b�g�̃x�N�g��
		m_vec[0] = (pl[0].pos - pl[0].ppos) / 10.f;
		m_vec[1] = (pl[1].pos - pl[1].ppos) / 10.f;

		D2D1_POINT_2F d[2] = {
			p_pos - pl[0].pos,
			p_pos - pl[1].pos
		};
		// �}���b�g�ƃp�b�N�̋�����2��
		FLOAT r2[2] = {
			d[0].x * d[0].x + d[0].y * d[0].y,
			d[1].x * d[1].x + d[1].y * d[1].y
		};

		// �p�b�N�ƃ}���b�g�̓����蔻��
		for(int i = 0; i < 2; ++i){
			if(r2[i] < (_this->data.m_r + _this->data.p_r) * (_this->data.m_r + _this->data.p_r)){
				// �}���b�g�ƃp�b�N���Փ˂������̉���炷
				_this->sounds[Col1_1 + i]->Play();
				if(d[i].x >= 0 && d[i].y <= 0){						//�E��
					if(p_vec.x <= 0 && p_vec.y >= 0){
						p_vec.x = -p_vec.x;
						p_vec.y = -p_vec.y;
					}else if(p_vec.x <= 0 && p_vec.y <= 0)
						p_vec.x = -p_vec.x;
					else if(p_vec.x >= 0 && p_vec.y >= 0)
						p_vec.y = -p_vec.y;

					p_vec += m_vec[i];
				}
				if(d[i].x < 0 && d[i].y < 0){
					if(p_vec.x >= 0 && p_vec.y >= 0){		//����
						p_vec.x = -p_vec.x;
						p_vec.y = -p_vec.y;
					}else if(p_vec.x <= 0 && p_vec.y >= 0)
						p_vec.y = -p_vec.y;
					else if(p_vec.x >= 0 && p_vec.y <= 0)
						p_vec.x = -p_vec.x;

					p_vec += m_vec[i];
				}
				if(d[i].x <= 0 && d[i].y >= 0){						//����
					if(p_vec.x >= 0 && p_vec.y <= 0){
						p_vec.x = -p_vec.x;
						p_vec.y = -p_vec.y;
					}else if(p_vec.x >= 0 && p_vec.y >= 0)
						p_vec.x = -p_vec.x;
					else if(p_vec.x <= 0 && p_vec.y <= 0)
						p_vec.y = -p_vec.y;

					p_vec += m_vec[i];
				}
				if(d[i].x > 0 && d[i].y > 0){						//�E��
					if(p_vec.x <= 0 && p_vec.y <= 0){
						p_vec.x = -p_vec.x;
						p_vec.y = -p_vec.y;
					}else if(p_vec.x >= 0 && p_vec.y <= 0)
						p_vec.y = -p_vec.y;
					else if(p_vec.x <= 0 && p_vec.y >= 0)
						p_vec.x = -p_vec.x;

					p_vec += m_vec[i];
				}
			}
		}

		// �p�b�N�̑��x����
		if(p_vec.x > 10.0 || p_vec.x < -10.0)
			p_vec.x = p_vec.x - p_vec.x/100;
		else if(p_vec.y > 10.0 || p_vec.y < -10.0)
			p_vec.y = p_vec.y - p_vec.y/100;
		else if(p_vec.x > -1.5 && p_vec.x < 1.5)
			p_vec.x = p_vec.x + p_vec.x/100;
		else if(p_vec.y > -1.5 && p_vec.y < 1.5)
			p_vec.y = p_vec.x + p_vec.x/100;

		// �p�b�N�ƕǂ̓����蔻��
		// ���܂��͉E�ɓ���������x�������x�̕����𔽓]������
		if(p_pos.y - p_r < (field.bottom - field.top)*2/7+field.top){		//����̕�
			if (p_pos.x < field.left + p_r || p_pos.x > field.right - p_r) {
				_this->sounds[Col2]->Play();
				p_vec.x = -p_vec.x;
			}
		}else if(p_pos.y + p_r > (field.bottom - field.top)*5/7+field.top){	//�E��̕�
			if (p_pos.x < field.left + p_r || p_pos.x > field.right - p_r) {
				_this->sounds[Col2]->Play();
				p_vec.x = -p_vec.x;
			}
		}else if(p_pos.y - p_r > (field.bottom - field.top)*2/7+field.top && p_pos.y + p_r < (field.bottom - field.top)*5/7+field.top){
			if (p_pos.x > field.right - p_r){
				// 2P���̃S�[���ɓ�����
				_this->sounds[Goal]->Play();
				::EnterCriticalSection(&_this->cs);
				pl[0].score++;
				_this->ResetPack(0);
				p_vec = D2D1::Point2F();
				::LeaveCriticalSection(&_this->cs);

				if(pl[0].score == _this->data.setscore){
					// 1P��3�_�����
					::PostMessageW(_this->hwnd, WM_APP_GAME_OVER, 0, 0);
				}
			}else if (p_pos.x < field.left + p_r){
				// 1P���̃S�[��
				_this->sounds[Goal]->Play();
				::EnterCriticalSection(&_this->cs);
				pl[1].score++;
				_this->ResetPack(1);
				p_vec = D2D1::Point2F();
				::LeaveCriticalSection(&_this->cs);

				if(pl[1].score == _this->data.setscore){
					// 2P��3�_�����
					::PostMessageW(_this->hwnd, WM_APP_GAME_OVER, 0, 0);
				}
			}
		}
				
		// ��܂��͉��ɓ���������y�������x�̕����𔽓]������
		if (p_pos.y - p_r < field.top || p_pos.y + p_r > field.bottom) {
			_this->sounds[Col2]->Play();
			p_vec.y = -p_vec.y;
		}

		::EnterCriticalSection(&_this->cs);
		p_pos += p_vec;
		pl[0].ppos = pl[0].pos;
		pl[1].ppos = pl[1].pos;
		::LeaveCriticalSection(&_this->cs);

		::InvalidateRect(_this->hwnd, NULL, FALSE);
	}

	return 0;
}
